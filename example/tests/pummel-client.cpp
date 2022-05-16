#include "dbus.h"
#include <chrono>
#include <iostream>
#include <mutex>

// Invoke:
//    pummel-client [stubname] [iterations_per_thread] [threads]

using namespace std::chrono;

size_t gTotalSuccesses = 0;
std::mutex gMutexOutput;
std::mutex gMutexSuccessCount;

bool testClient(const std::string& stubname, size_t iterations)
{
    DBus::Log::setLevel(DBus::Log::INFO);

    DBus::asio::io_context ioc;
    DBus::Connection::Ptr dbus = DBus::Connection::create(ioc);
    if (!dbus)
        return false;

    milliseconds ms_start = duration_cast<milliseconds>(system_clock::now().time_since_epoch());

    size_t replies = 0;
    size_t correct = 0;
    size_t errors = 0;
    size_t sent = 0;

    milliseconds timeout = std::chrono::seconds(45);
    DBus::asio::steady_timer timer(ioc);
    timer.expires_after(timeout);
    timer.async_wait(
        [dbus]
        (const DBus::error_code& error)
        {
            if (!error)
                dbus->disconnect();
        });

    dbus->connect(
        DBus::Platform::getSessionBus(),
        DBus::AuthenticationProtocol::create(),
        [dbus, &timer, &errors, &replies, &correct, &sent, &stubname, iterations]
        (const DBus::Error& error, const std::string&, const std::string&) {
            if (error)
                return;

            while (sent < iterations) {
                const std::string number(std::to_string(sent++));
                const std::string expected(stubname + ' ' + number);

                dbus->sendMethodCall(
                    { "biz.brightsign",
                      {"/", "biz.brightsign.test", "concat"}, {stubname, number} },
                    [dbus, &timer, &errors, &replies, &correct, expected, iterations]
                    (const DBus::Error& error, const DBus::Message::MethodReturn& reply) {
                        if (error)
                            ++errors;
                        else {
                            ++replies;
                            const auto result = reply.getParameter(0).asString();
                            if (result == expected)
                                ++correct;
                        }

                        if (errors + replies == iterations) {
                            timer.cancel();
                            dbus->disconnect();
                        }
                    });

                // If we have a short delay (e.g. 1ms) between messages, everything is happy
                // for 10-10-100 If this is 10ms, then the timeouts below happen a lot more
                // frequently. ATM, I can't tell if this is coincidence, or intentional.
                // std::this_thread::sleep_for(message_sleep);
            }
        });

    ioc.run();

    milliseconds ms_end = duration_cast<milliseconds>(system_clock::now().time_since_epoch());

    std::lock_guard<std::mutex> guard(gMutexOutput);
    std::cout << (correct == sent ? "SUCCESS" : "FAILURE");
    std::cout << " " << stubname << ":: Results ::";
    std::cout << "  Sent: " << sent;
    std::cout << "  Replies: " << replies;
    std::cout << "  Correct: " << correct;
    std::cout << "  Errors: " << errors;
    std::cout << "  Duration: " << (ms_end - ms_start).count() << "ms";
    std::cout << std::endl;

    std::cout << "  Parameters: " << std::endl;
    std::cout << "    Timeout: " << timeout.count() << "ms" << std::endl;
    std::cout << "    Iterations: " << iterations << std::endl;

    std::cout << dbus->getStats() << std::endl;

    return correct == sent;
}

void threadTestClient(const std::string& stubname, int iterations)
{
    if (testClient(stubname, iterations)) {
        std::lock_guard<std::mutex> guard(gMutexSuccessCount);
        ++gTotalSuccesses;
    }
}

int main(int argc, const char* argv[])
{
    std::vector<std::thread> threads;
    std::string stubname(argc < 2 ? std::string("TEST") : (argv[1]));
    size_t iterations = argc < 3 ? 100 : atoi(argv[2]);
    size_t numThreads = argc < 4 ? 10 : atoi(argv[3]);

    for (size_t i = 0; i < numThreads; ++i) {
        std::string fullname(stubname);
        fullname += "_thread_";
        fullname += std::to_string(i);
        threads.emplace_back(std::thread(threadTestClient, fullname, iterations));
    }

    for (auto& th : threads) {
        th.join();
    }

    return gTotalSuccesses == numThreads ? 0 : -1;
}
