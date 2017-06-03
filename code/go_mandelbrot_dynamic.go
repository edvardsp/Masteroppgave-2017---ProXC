package main
import (
	"fmt"
	"sync"
	"time"
)
const (
	ROUNDS = 100
	MAX_ITER = 255
	/* x in [-2.1; 1.0] */
	/* y in [-1.3; 1.3] */
	XMIN = -2.1
	XMAX =  1.0
	YMIN = -1.3
	YMAX =  1.3
)

type MandelbrotData struct {
	line uint
	values []float64
}

func point_predicate(x, y float64) bool {
	return (x * x + y * y) < 4.0
}

func mandelbrot(wg *sync.WaitGroup, line, dim uint, data_ch chan<- MandelbrotData) {
	integral_x := (XMAX - XMIN) / float64(dim);
	integral_y := (YMAX - YMIN) / float64(dim);
	data := MandelbrotData{ line, make([]float64, dim) }
	y := YMIN + float64(line) * integral_y
	x := XMIN
	for x_coord := uint(0); x_coord < dim; x_coord++ {
		x1 := 0.0
		y1 := 0.0
		loop_count := 0
		for loop_count < MAX_ITER && point_predicate(x1, y1) {
			loop_count += 1
			x1_new := x1 * x1 - y1 * y1 + x
			y1 = 2.0 * x1 * y1 + y
			x1 = x1_new
		}
		value := float64(loop_count) / float64( MAX_ITER )
		data.values[x_coord] = value
		x += integral_x
	}
	data_ch <- data
	wg.Done()
}

func consumer(wg *sync.WaitGroup, dim uint, data_ch <-chan MandelbrotData) {
	results := make([][]float64, dim)
	for i := uint(0); i < dim; i++ {
		data := <-data_ch
		results[data.line] = data.values
	}
	wg.Done()
}

func mandelbrot_program(dim uint) {
	sum := int64(0)
	for round := 0; round < ROUNDS; round++ {
		data_ch := make(chan MandelbrotData)
		start := time.Now()
		var wg sync.WaitGroup
		wg.Add( 1 + int(dim) )
		go consumer(&wg, dim, data_ch)
		for line := uint(0); line < dim; line++ {
			go mandelbrot(&wg, line, dim, data_ch)
		}
		wg.Wait()
		elapsed := time.Since(start)
		sum += elapsed.Nanoseconds()
	}
	fmt.Println(dim, ",", sum/ROUNDS)
}

func main() {
	for dim := uint(1); dim < 100; dim += 1 {
		mandelbrot_program(dim)
	}
	for dim := uint(100); dim < 500; dim += 5 {
		mandelbrot_program(dim)
	}
	for dim := uint(500); dim <= 1000; dim += 10 {
		mandelbrot_program(dim)
	}
}
