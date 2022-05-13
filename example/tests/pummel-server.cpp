#include "dbus.h"
#include <iostream>

void serveIntrospect(DBus::Connection::Ptr dbus, const std::string& xml)
{
    dbus->receiveMethodCall(
        "org.freedesktop.DBus.Introspectable.Introspect",
        [dbus, &xml](const DBus::Message::MethodCall& call) {
            DBus::Message::MethodReturn reply(call.getSender(), call.getSerial());
            reply.addParameter(DBus::Type::String(xml));
            dbus->sendMethodReturn(reply,
                [dbus, &xml](const DBus::Error& error) {
                    if (!error)
                        serveIntrospect(dbus, xml);
                });
        });

}

// It is recommend you implement org.freedesktop.DBus.Properties.GetAll,
// even if you only return an empty string. Some apps (like d-feet) will
// issue a GetAll before the standard method call you've requested, and
// then wait for a reply, making them appear slow.
void serveGetAll(DBus::Connection::Ptr dbus)
{
    dbus->receiveMethodCall(
        "org.freedesktop.DBus.Properties.GetAll",
        [dbus](const DBus::Message::MethodCall& call) {
            DBus::Message::MethodReturn reply(call.getSender(), call.getSerial());
            reply.addParameter(DBus::Type::String(""));
            dbus->sendMethodReturn(reply,
                [dbus](const DBus::Error& error) {
                    if (!error)
                        serveGetAll(dbus);
                });
        });
}

void serveConcat(DBus::Connection::Ptr dbus)
{
    dbus->receiveMethodCall(
        "biz.brightsign.test.concat",
        [dbus](const DBus::Message::MethodCall& call) {
            if (call.isReplyExpected()) {
                if (call.getParameterCount() != 2) {
                    DBus::Message::Error error(
                        call.getSender(), call.getSerial(),
                        "biz.brightsign.Error.InvalidParameters",
                        "This needs 2 params.");

                    dbus->sendError(
                        error,
                        [dbus](const DBus::Error& error){
                            if (error) {
                                std::cerr << "sendError failed: " << error.message << std::endl;
                                dbus->disconnect();
                            }
                        });
                } else {
                    const auto input1 = call.getParameter(0).asString();
                    const auto input2 = call.getParameter(1).asString();
                    DBus::Message::MethodReturn reply(call.getSender(), call.getSerial());
                    reply.addParameter(DBus::Type::String(input1 + " " + input2));

                    dbus->sendMethodReturn(reply,
                        [dbus](const DBus::Error& error) {
                            if (error) {
                                std::cerr << "concat failed: " << error.message << std::endl;
                                dbus->disconnect();
                            }
                        });
                }
            }
            serveConcat(dbus);
        });
}

int main()
{
    DBus::Log::setLevel(DBus::Log::WARNING);

    DBus::asio::io_context ioc;
    auto dbus = DBus::Connection::create(ioc);
    if (!dbus)
        return 1;

    DBus::Introspectable::Interface iface("biz.brightsign.test");
    iface.addMethod(DBus::Introspectable::Method("concat", "ss", "s"));
    DBus::Introspectable::Introspection introspection;
    introspection.addInterface(iface);

    dbus->connect(
        DBus::Platform::getSessionBus(),
        DBus::AuthenticationProtocol::create(),
        [dbus, &introspection]
        (const DBus::Error& error, const std::string&, const std::string&) {
            if (error)
                return;

            serveIntrospect(dbus, introspection.serialize());
            serveGetAll(dbus);
            serveConcat(dbus);

            dbus->requestName(
                "biz.brightsign", DBus::RequestNameFlag::None,
                [dbus](const DBus::Error& error, const DBus::Message::MethodReturn& reply) {
                    if (error) {
                        std::cerr << "RequestName error: " << error.message << std::endl;
                        dbus->disconnect();
                        return;
                    }

                    auto result = DBus::Type::asUint32(reply.getParameter(0));
                    std::cout << "Now running as biz.brightsign (" << result << ")" << std::endl;
                    if (result != DBus::RequestNameReply::PrimaryOwner)
                        std::cout << "Not Primary Owner." << std::endl;
                });
        });

    DBus::asio::signal_set signals(ioc, SIGINT, SIGTERM);
    signals.async_wait(
        [dbus]
        (const DBus::error_code& error, int /*signal_number*/) {
            if (!error)
                dbus->disconnect();
        });

    // Now do a mini-self test
    dbus->sendMethodCall(
        { "biz.brightsign", {"/", "biz.brightsign.test", "concat"}, {"one", "two"} },
        [dbus](const DBus::Error& error, const DBus::Message::MethodReturn& reply) {
            if (error) {
                std::cout
                    << "Terminating because concat has returned an error"
                    << std::endl;
                dbus->disconnect();
            }
            const auto result = reply.getParameter(0).asString();
            if (result != "one two") {
                std::cout
                    << "Terminating because concat is returning incorrect base data"
                    << std::endl;
                dbus->disconnect();
            }
        });

    ioc.run();

//TODO     std::cout << native.getStats();

    return 0;
}
