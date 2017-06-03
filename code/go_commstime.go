package main
import (
	"fmt"
	"time"
	"sync"
)
const (
	REPEAT = 100
	RUNS = 50
)
func chainer(wg *sync.WaitGroup, in <-chan uint, out chan<- uint) {
	for item := range in {
		out <- item
	}
	close(out)
	wg.Done()
}

func prefix(wg *sync.WaitGroup, in <-chan uint, out chan<- uint) {
	out <- 0
	for item := range in {
		out <- item + 1
	}
	close(out)
	wg.Done()
}

func delta(wg *sync.WaitGroup, in <-chan uint, out, out_consume chan<- uint, ex chan bool) {
	running := true
	for running {
		item := <-in
		select {
		case out_consume <- item:
			out <- item
		case <-ex:
			close(out)
			running = false
		}
	}
	wg.Done()
}

func consumer(wg *sync.WaitGroup, in_consume <-chan uint, ex chan bool) {
	for i := 0; i < REPEAT; i++ {
		<-in_consume
	}
	close(ex)
	wg.Done()
}

func commstime(chain int) {
	sum := int64(0)
	for run := 0; run < RUNS; run++ {
		var wg sync.WaitGroup
		wg.Add(3 + chain)
		del2pre_ch := make(chan uint)
		consume_ch := make(chan uint)
		chain_chs := make([]chan uint, chain + 1)
		for i := range chain_chs {
			chain_chs[i] = make(chan uint)
		}
		ex_ch := make(chan bool)
		start := time.Now()
		go prefix(&wg,   chain_chs[0], del2pre_ch)
		go delta(&wg,    del2pre_ch,   chain_chs[chain], consume_ch, ex_ch)
		go consumer(&wg, consume_ch,   ex_ch)
		for i := 0; i < chain; i++ {
			go chainer(&wg, chain_chs[i + 1], chain_chs[i])
		}
		wg.Wait()
		sum += time.Since(start).Nanoseconds()
	}
	fmt.Println(chain, ",", sum/RUNS)
}

func main() {
	for chain := 1; chain < 50; chain += 1 {
		commstime(chain)
	}
	for chain := 50; chain < 500; chain += 5 {
		commstime(chain)
	}
	for chain := 500; chain <= 1000; chain += 10 {
		commstime(chain)
	}
}
