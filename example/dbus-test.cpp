
/*
DBus-
      :::::::::      ::::::::::     :::::::::       ::::::::   :::::::::::
     :+:    :+:     :+:            :+:    :+:     :+:    :+:      :+:
    +:+    +:+     +:+            +:+    +:+     +:+    +:+      +:+
   +#+    +:+     +#++:++#       +#++:++#+      +#+    +:+      +#+
  +#+    +#+     +#+            +#+            +#+    +#+      +#+
 #+#    #+#     #+#            #+#            #+#    #+#      #+#
#########      ##########     ###             ########       ###

*/

/*
https://dbus.freedesktop.org/doc/dbus-specification.html


ASIO learnings from:
https://www.gamedev.net/blogs/entry/2249317-a-guide-to-getting-started-with-boostasio/

SASL (w/dbus)
https://github.com/bus1/dbus-broker/blob/master/src/dbus/test-sasl.c

About unix_fd
https://lists.freedesktop.org/archives/dbus/2012-November/015341.html
*/

#include "dbus.h"
#include <iostream>

void handleNameAcquired(DBus::Connection::Ptr dbus)
{
    dbus->receiveSignal(
        "org.freedesktop.DBus.NameAcquired",
        [dbus](const DBus::Message::Signal& signal) {
            if (!signal) return;
            // Knowing the type is 's' we can be safe in assuming asString will work
            const auto name = DBus::Type::asString(signal.getParameter(0));
            std::cout << "RCV signal : NameAcquired : " << name << std::endl;
            handleNameAcquired(dbus);
        });
}

void handleNameOwnerChanged(DBus::Connection::Ptr dbus)
{
    dbus->receiveSignal(
        "org.freedesktop.DBus.NameOwnerChanged",
        [dbus](const DBus::Message::Signal& signal) {
            if (!signal) return;
            const auto name = DBus::Type::asString(signal.getParameter(0));
            std::cout << "RCV signal : NameOwnerChanged : " << name << std::endl;
            handleNameOwnerChanged(dbus);
        });
}

void servePropertiesGet(DBus::Connection::Ptr dbus)
{
    dbus->receiveMethodCall(
        "org.freedesktop.DBus.Properties.Get",
        [dbus](const DBus::Message::MethodCall& call) {
            if (!call) return;
            const auto interface = call.getParameter(0).asString();
            const auto property = call.getParameter(1).asString();

            auto errorHandler = [](const DBus::Error& error){
                if (error)
                    std::cerr << "error while sending Get reply:" << error.message << std::endl;
            };

            if (property == "Answer") {
                DBus::Message::MethodReturn reply(call.getSender(), call.getSerial());
                reply.addParameter(DBus::Type::Uint16(42));
                dbus->sendMethodReturn(reply, errorHandler);
            } else if (property == "Poetry") {
                DBus::Message::MethodReturn reply(call.getSender(), call.getSerial());
                reply.addParameter(DBus::Type::String("The dead swans lay in the stagnant pool."));
                dbus->sendMethodReturn(reply, errorHandler);
            } else {
                DBus::Message::Error err(call.getSender(), call.getSerial(),
                    "biz.brightsign.Error.InvalidParameters",
                    "Requested property is unknown: " + property);
                dbus->sendError(err, errorHandler);
            }
            servePropertiesGet(dbus);
        });
}

void servePropertiesGetAll(DBus::Connection::Ptr dbus)
{
    // TODO: finish org.freedesktop.DBus.Properties.GetAll to return the correct
    // data as name:value pair looks wrong, but it doesn't crash. Whereas the a(sv)
    // is right, but causes the sender to terminate the connection
    dbus->receiveMethodCall(
        "org.freedesktop.DBus.Properties.GetAll",
        [dbus](const DBus::Message::MethodCall& method) {
            if (!method) return;
            printf("GetAll\nCALLBACK METHOD : serial %.4x \n", method.getSerial());
            printf("CALLBACK METHOD : sender %s \n",
                method.getSender().c_str());
            printf("CALLBACK METHOD : destination %s \n",
                method.getDestination().c_str());
            printf("CALLBACK METHOD : serial %d \n", method.getSerial());

            DBus::Type::Struct p1;
            p1.add(DBus::Type::String("Answer"));
            p1.add(DBus::Type::Variant(DBus::Type::Uint16(42)));

            DBus::Type::Struct p2;
            p2.add(DBus::Type::String("Poetry"));
            p2.add(DBus::Type::Variant(DBus::Type::String("The dead swans lay in the stagnant pool.")));

            DBus::Type::Array properties;
            properties.add(p1);

            DBus::Message::MethodReturn reply(method.getSender(), method.getSerial());
            reply.addParameter(properties);
            dbus->sendMethodReturn(reply,
                [](const DBus::Error& error){
                    if (error)
                        std::cerr << "error while sending GetAll reply:" << error.message << std::endl;
                });
            servePropertiesGetAll(dbus);
        });
}

void serveIntrospect(DBus::Connection::Ptr dbus)
{
    static std::string xml;
    if (xml.empty())
    {
        DBus::Introspectable::Interface iface("biz.brightsign.TestInterface");
        iface.addMethod(DBus::Introspectable::Method("Ping", "", "ss"));
        iface.addMethod(DBus::Introspectable::Method("Echo2", "ss", "s"));
        iface.addProperty(DBus::Introspectable::Property("Answer", "q"));
        iface.addProperty(DBus::Introspectable::Property("Poetry", "s"));
        iface.addSignal(DBus::Introspectable::Signal("BroadcastStuff", "s"));

        DBus::Introspectable::Introspection introspection;
        introspection.addInterface(iface);
        xml = introspection.serialize();
    }

    dbus->receiveMethodCall(
        "org.freedesktop.DBus.Introspectable.Introspect",
        [dbus](const DBus::Message::MethodCall& call) {
            if (!call) return;
            DBus::Message::MethodReturn reply(call.getSender(), call.getSerial());
            reply.addParameter(DBus::Type::String(xml));
            dbus->sendMethodReturn(reply,
                [](const DBus::Error& error){
                    if (error)
                        std::cerr << "error serving introspection: "
                                  << error.message << std::endl;
                });
            serveIntrospect(dbus);
        });
}

void servePing(DBus::Connection::Ptr dbus)
{
    dbus->receiveMethodCall(
        "biz.brightsign.TestInterface.Ping",
        [dbus](const DBus::Message::MethodCall& call) {
            if (!call) return;
            DBus::Message::MethodReturn reply(call.getSender(), call.getSerial());
            reply.addParameter(DBus::Type::String("Ping??"));
            reply.addParameter(DBus::Type::String("Pong!!"));
            dbus->sendMethodReturn(reply,
                [](const DBus::Error& error) {
                    if (error)
                        std::cerr << "error while sending Ping reply:" << error.message << std::endl;
                });
            servePing(dbus);
        });
}

void serveEcho2(DBus::Connection::Ptr dbus)
{
    dbus->receiveMethodCall(
        "biz.brightsign.TestInterface.Echo2",
        [dbus](const DBus::Message::MethodCall& call) {
            if (!call) return;
            if (call.isReplyExpected()) {
                auto errorHandler = [](const DBus::Error& error) {
                    if (error)
                        std::cerr << "error while sending Echo2 reply: " << error.message << std::endl;
                };
                if (call.getParameterCount() != 2) {
                    DBus::Message::Error err(call.getSender(), call.getSerial(),
                        "biz.brightsign.Error.InvalidParameters",
                        "This needs 2 params.");
                    dbus->sendError(err, errorHandler);
                } else {
                    const auto input1(call.getParameter(0).asString());
                    const auto input2(call.getParameter(1).asString());
                    DBus::Message::MethodReturn reply(call.getSender(), call.getSerial());
                    reply.addParameter(DBus::Type::String("Echo of : " + input1 + " and " + input2));
                    dbus->sendMethodReturn(reply, errorHandler);
                }
            }
            serveEcho2(dbus);
        });
}

void serveOpenFile(DBus::Connection::Ptr dbus)
{
    dbus->receiveMethodCall(
        "biz.brightsign.TestInterface.OpenFile",
        [dbus](const DBus::Message::MethodCall& call) {
            if (!call) return;
            if (call.isReplyExpected()) {
                auto errorHandler = [](const DBus::Error& error) {
                    if (error)
                        std::cerr << "error while sending OpenFile reply: " << error.message << std::endl;
                };
                if (call.getParameterCount() != 1) {
                    DBus::Message::Error err(call.getSender(), call.getSerial(),
                        "biz.brightsign.Error.InvalidParameters",
                        "This needs 2 params.");
                    dbus->sendError(err, errorHandler);
                } else {
                    const auto input1 = DBus::Type::asString(call.getParameter(0));
                    DBus::Message::MethodReturn reply(call.getSender(), call.getSerial());
                    int fd = 0;
                    reply.addParameter(DBus::Type::UnixFd(fd));
                    dbus->sendMethodReturn(reply, errorHandler);
                }
            }
            serveOpenFile(dbus);
        });
}

void test1()
{
    DBus::Log::setLevel(DBus::Log::TRACE);

    DBus::Log::write(DBus::Log::INFO, "System bus: %s\n",
        DBus::Platform::getSystemBus().c_str());
    DBus::Log::write(DBus::Log::INFO, "Session bus: %s\n",
        DBus::Platform::getSessionBus().c_str());

    DBus::asio::io_context ioc;
    DBus::Connection::Ptr dbus = DBus::Connection::create(ioc);
    if (!dbus)
        return;

    dbus->connectSessionBus(
        [dbus](const DBus::Error& error, const std::string&, const std::string&) {
            if (error)
                return;

            handleNameAcquired(dbus);
            handleNameOwnerChanged(dbus);

            servePropertiesGetAll(dbus);
            servePropertiesGet(dbus);
            serveIntrospect(dbus);

            servePing(dbus);
            serveEcho2(dbus);
            serveOpenFile(dbus);

            dbus->requestName(
                "test.steev", DBus::RequestNameFlag::None,
                [dbus](const DBus::Error& error, const DBus::Message::MethodReturn& msg) {
                    if (error)
                        return dbus->disconnect();

                    uint32_t result = DBus::Type::asUint32(msg.getParameter(0));
                    if (result != DBus::RequestNameReply::PrimaryOwner)
                        std::cerr << "Not Primary Owner" << std::endl;

                    std::cout << "Try and call ourselves..." << std::endl;
                    dbus->sendMethodCall(
                        { "test.steev",
                          { "/", "biz.brightsign.TestInterface", "Echo2" }, {"one", "two"} },
                        [](const DBus::Error& error, const DBus::Message::MethodReturn& reply) {
                            const auto result = reply.getParameter(0).asString();
                            std::cout << "REPLY FROM Echo2 : " << result << std::endl;
                        });

                    dbus->sendMethodCall(
                        { "test.steev",
                          { "/", "biz.brightsign.TestInterface", "OpenFile" }, {"/etc/passwd"} },
                        [dbus](const DBus::Error& error, const DBus::Message::MethodReturn& reply) {
                            const auto result = DBus::Type::asUnixFd(reply.getParameter(0));
                            std::cout << "REPLY FROM OpenFile : " << result << std::endl;
                            dbus->disconnect();

                            std::cout << '\n' << dbus->getStats();
                        });
                });
        });

    ioc.run();
}

int main()
{
    test1();
    return 0;
}
