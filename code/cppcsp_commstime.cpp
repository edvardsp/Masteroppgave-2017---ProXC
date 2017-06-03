#include <chrono>
#include <functional>
#include <iostream>
#include <sstream>
#include <vector>
#include "csp.h"

using namespace csp;
using Chan = one2one_chan< std::size_t >;
using Tx = chan_out< std::size_t >;
using Rx = chan_in< std::size_t >;

void chainer( Rx in, Tx out ) {
    for ( ;; ) {
        out( in() );
    }
}

void prefix( Rx in, Tx out ) {
    out( 0 );
    for ( ;; ) {
        out( 1 + in() );
    }
}

void delta( Rx in, Tx out, Tx out_consume ) {
    for ( ;; ) {
        std::size_t item = in();
        out( item );
        out_consume( item );
    }
}

int main( int argc, char *argv[] ) {
    if ( argc != 2 ) { return 1; }
    std::size_t chain;
    std::stringstream{ argv[1] } >> chain;
    Chan consume_ch, pre2del_ch;
    std::vector< Chan > chain_chs( chain + 1 );
    std::vector< std::function< void() > > in_chain;
    in_chain.reserve( chain );
    for ( std::size_t i = 0; i < chain; ++i ) {
        in_chain.push_back( make_proc( chainer,
            chain_chs[i+1], chain_chs[i] ) );
    }
    auto start = std::chrono::system_clock::now();
    par{
        par{ in_chain },
        make_proc( prefix,    chain_chs[0], pre2del_ch ),
        make_proc( delta,     pre2del_ch, chain_chs[chain], consume_ch ),
        make_proc([&]( Rx in_consume ){
            constexpr std::size_t repeat = 100;
            for ( std::size_t i = 0; i < repeat; ++i ) {
                in_consume();
            }
            auto end = std::chrono::system_clock::now();
            std::chrono::duration< std::size_t, std::nano > diff = end - start;
            std::cout << chain << "," << diff.count() << std::endl;
            std::exit(0);
        }, consume_ch)
    }();
    return 0;
}
