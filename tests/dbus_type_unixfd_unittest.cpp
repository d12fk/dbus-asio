#include "dbus_messageistream.h"
#include "dbus_messageostream.h"
#include "dbus_type_unixfd.h"
#include <catch2/catch.hpp>

#include <fcntl.h>
#include <unistd.h>

namespace DBus {
namespace test {

using Catch::Matchers::Equals;

    TEST_CASE("Marshall and unmarshall unix fd")
    {
        int fd = ::open("/tmp/unixfd_unittest", O_CREAT, S_IRUSR);
        CHECK(fd != -1);

        Type::UnixFd unixFdOut(fd);
        ::close(fd); // unixFdOut owns a dup(2) of fd at this point
        ::unlink("/tmp/unixfd_unittest");

        MessageOStream ostream;
        unixFdOut.marshall(ostream);
        REQUIRE(ostream.fds.size() == 1);

        OctetBuffer buf(
            reinterpret_cast<const uint8_t*>(ostream.data.data()),
            ostream.data.size(),
            ostream.fds);
        MessageIStream istream(buf, false);
        Type::UnixFd unixFdIn;
        unixFdIn.unmarshall(istream);

        // UnixFds should reference the same file, but with different fds
        REQUIRE_THAT(unixFdOut.fdName(), Equals(unixFdIn.fdName()));
        REQUIRE_THAT(unixFdOut.asString(), !Equals(unixFdIn.asString()));

        // UnixFd::dup() should produce yet another fd to the file
        int dup = unixFdIn.dup();
        REQUIRE_THAT(std::to_string(dup), !Equals(unixFdIn.asString()));
        ::close(dup);
    }


} // namespace test
} // namespace DBus
