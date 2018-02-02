//------------------------------------------------------------------------------
/*
    This file is part of Beast: https://github.com/vinniefalco/Beast
    Copyright 2013, Vinnie Falco <vinnie.falco@gmail.com>



*/
//==============================================================================

#include <sdchain/beast/hash/endian.h>
#include <sdchain/beast/hash/fnv1a.h>
#include <sdchain/beast/hash/siphash.h>
#include <sdchain/beast/hash/xxhasher.h>
#include <sdchain/beast/xor_shift_engine.h>
#include <sdchain/beast/unit_test.h>
#include <array>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <random>

namespace beast {

class hash_speed_test : public beast::unit_test::suite
{
public:
    template <class Generator>
    static
    void
    rngfill (void* buffer, std::size_t bytes,
        Generator& g)
    {
        using result_type =
            typename Generator::result_type;
        while (bytes >= sizeof(result_type))
        {
            auto const v = g();
            std::memcpy(buffer, &v, sizeof(v));
            buffer = reinterpret_cast<
                std::uint8_t*>(buffer) + sizeof(v);
            bytes -= sizeof(v);
        }
        if (bytes > 0)
        {
            auto const v = g();
            std::memcpy(buffer, &v, bytes);
        }
    }

    template <class Generator, std::size_t N,
        class = std::enable_if_t<
            N % sizeof(typename Generator::result_type) == 0>>
    static
    void
    rngfill (std::array<std::uint8_t, N>& a, Generator& g)
    {
        using result_type =
            typename Generator::result_type;
        auto i = N / sizeof(result_type);
        result_type* p =
            reinterpret_cast<result_type*>(a.data());
        while (i--)
            *p++ = g();
    }
    using clock_type =
        std::chrono::high_resolution_clock;
    template <class Hasher, std::size_t KeySize>
    void
    test (std::string const& what, std::size_t n)
    {
        using namespace std;
        using namespace std::chrono;
        xor_shift_engine g(1);
        array<std::uint8_t, KeySize> key;
        auto const start = clock_type::now();
        while(n--)
        {
            rngfill (key, g);
            Hasher h;
            h(key.data(), KeySize);
            volatile size_t temp =
                static_cast<std::size_t>(h);
            (void)temp;
        }
        auto const elapsed = clock_type::now() - start;
        log << setw(12) << what << " " <<
            duration<double>(elapsed).count() << "s" << std::endl;
    }

    void
    run()
    {
        enum
        {
            N = 100000000
        };

    #if ! BEAST_NO_XXHASH
        test<xxhasher,32>   ("xxhash", N);
    #endif
        test<fnv1a,32>      ("fnv1a", N);
        test<siphash,32>    ("siphash", N);
        pass();
    }
};

BEAST_DEFINE_TESTSUITE_MANUAL(hash_speed,container,beast);

} // beast
