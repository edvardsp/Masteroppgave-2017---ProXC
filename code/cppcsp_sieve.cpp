#include <chrono>
#include <iostream>
#include <functional>
#include <vector>
#include "csp.h"

using ClockT = std::chrono::steady_clock;
using TimePointT = typename ClockT::time_point;
using Chan = csp::one2one_chan< long >;

void generate( Chan out ) {
    long i = 2;
    for ( ;; ) {
        out( i++ );
    }
}

void filter( Chan in, Chan out ) {
    long prime = in();
    for ( ;; ) {
        long i = in();
        if ( i % prime != 0 ) {
            out( i );
        }
    }
}

void time_taker( std::size_t n, TimePointT start, Chan in ) {
    long prime = in();
    auto end = ClockT::now();
    std::chrono::duration< std::size_t, std::nano > diff = end - start;
    std::cout << n << "," << diff.count() / n << std::endl;
    std::exit(0);
}

int main( int argc, char *argv[] ) {
    if ( argc != 2 ) { return 1; }
    std::size_t n;
    std::stringstream{ arg[1] } >> n;
    std::vector< Chan > chans( n );
    std::vector< std::function< void() > > procs;
    procs.reserve( n - 1 );
    for ( std::size_t i = 0; i < n - 1; ++i ) {
        procs.push_back( csp::make_proc(
            filter, chans[i], chans[i+1]
        ) );
    }
    auto start = ClockT::now();
    csp::par{
        make_proc( generate, chans[0] ),
        csp::par( procs ),
        make_proc( time_taker, n, start, chans[n-1] )
    }();
}
