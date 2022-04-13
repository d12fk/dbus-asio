#include "dbus_messageostream.h"
#include "dbus_messageprotocol.h"
#include "dbus_type_double.h"
#include "test_unmarshall.h"

namespace DBus {
namespace test {

    void TestUnmarshallFromMessageIStream(Type::Double& dbusType, double value,
        unsigned byteOrder)
    {
        uint64_t u;
        memcpy(&u, &value, sizeof(u));

        // __FLOAT_WORD_ORDER__ is the same as __BYTE_ORDER__ on
        // almost everything in the world. We only swap or not so we
        // couldn't cope if it is not. Let's use __FLOAT_WORD_ORDER__
        // if it's available so at least this test will fail in that
        // case.
#if defined(__FLOAT_WORD_ORDER__)
        if (byteOrder != __FLOAT_WORD_ORDER__)
            u = __bswap_64(u);
#else
        if (byteOrder != __BYTE_ORDER)
            u = __bswap_64(u);
#endif
        OctetBuffer buf((uint8_t*)&u, sizeof(u));
        MessageIStream stream(buf, byteOrder != __BYTE_ORDER);
        dbusType.unmarshall(stream);

        REQUIRE(static_cast<double>(dbusType) == value);
    }

    TEST_CASE("Unmarshall double little endian from MessageIStream")
    {
        Type::Double dbusType;
        TestUnmarshallFromMessageIStream(dbusType, 5346.12, __LITTLE_ENDIAN);
    }

    TEST_CASE("Unmarshall double big endian from MessageIStream")
    {
        Type::Double dbusType;
        TestUnmarshallFromMessageIStream(dbusType, 67.25, __BIG_ENDIAN);
    }

    TEST_CASE("Marshall and unmarshall double")
    {
        MessageOStream stream;
        double value = 1.2234;
        stream.writeDouble(value);

        Type::Double dbusType;
        OctetBuffer buf((uint8_t*)stream.data.data(), stream.size());
        MessageIStream istream(buf, false);
        dbusType.unmarshall(istream);

        REQUIRE(static_cast<double>(dbusType) == value);
    }

} // namespace test
} // namespace DBus

using namespace DBus::test;
