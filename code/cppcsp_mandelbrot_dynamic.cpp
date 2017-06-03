#include <chrono>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include "csp.h"

using namespace csp;

constexpr int NUM_WORKERS = 8;
constexpr std::size_t MAX_ITERATIONS = 255;
constexpr std::size_t ROUNDS = 100;
/* x in [-2.1; 1.0] */
/* y in [-1.3; 1.3] */
constexpr double xmin = -2.1;
constexpr double xmax = 1.0;
constexpr double ymin = -1.3;
constexpr double ymax = 1.3;

template<typename T>
using mobile = std::unique_ptr< T >;

struct mandelbrot_packet {
    int line = 0;
    std::vector< double > data;
};

inline bool point_predicate( const double x, const double y ) noexcept {
    return ( x * x + y * y ) < 2.0;
}

void mandelbrot(int line, int dim, chan_out< mobile< mandelbrot_packet > > out) noexcept {
    double integral_x = (xmax - xmin) / static_cast<double>(dim);
    double integral_y = (ymax - ymin) / static_cast<double>(dim);
    double x, y, x1, y1, xx = 0.0;
    std::size_t loop_count = 0;
    mobile< mandelbrot_packet > packet{ new mandelbrot_packet() };
    packet->line = line;
    packet->data = std::vector< double >( dim );
    y = ymin + line * integral_y;
    x = xmin;
    for ( std::size_t x_coord = 0; x_coord < dim; ++x_coord ) {
        x1 = 0.0;
        y1 = 0.0;
        loop_count = 0;
        while ( loop_count < MAX_ITERATIONS && point_predicate( x1, y1 ) ) {
            ++loop_count;
            xx = x1 * x1 - y1 * y1 + x;
            y1 = 2 * x1 * y1 + y;
            x1 = xx;
        }
        auto val = static_cast< double >( loop_count ) / static_cast< double >( MAX_ITERATIONS );
        packet->data[ x_coord ] = val;
        x += integral_x;
    }
    out( std::move( packet ) );
}

void consumer( chan_in< mobile< mandelbrot_packet > > in, int lines ) noexcept {
    std::vector< std::vector< double > > results(lines);
    for ( int i = 0; i < lines; ++i ) {
        auto packet = in();
        results[ packet->line ] = std::move( packet->data );
    }
}

void mandelbrot_program( std::size_t dim ) {
    any2one_chan< mobile< mandelbrot_packet > > data;
    std::vector< std::function< void() > > workers;
    for ( int i = 0; i < dim; ++i ) {
        workers.push_back(make_proc(mandelbrot, i, dim, data));
    }
    std::size_t sum = 0;
    for ( std::size_t i = 0; i < ROUNDS; ++i ) {
        auto start = system_clock::now();
        par {
            par(workers),
            make_proc(consumer, data, dim)
        }();
        auto stop = system_clock::now();
        std::chrono::duration< std::size_t, std::nano > diff = stop - start;
        sum += diff.count();
    }
    std::cout << dim << ", " << sum / ROUNDS << std::endl;
}

int main() {
    for ( std::size_t dim = 1; dim < 100; dim += 1 ) {
        mandelbrot_program( dim );
    }
    for ( std::size_t dim = 100; dim < 500; dim += 5 ) {
        mandelbrot_program( dim );
    }
    for ( std::size_t dim = 500; dim <= 1000; dim += 10 ) {
        mandelbrot_program( dim );
    }
    return 0;
}

