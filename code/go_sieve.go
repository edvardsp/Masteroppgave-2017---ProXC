package main
import (
	"fmt"
	"time"
	"os"
	"strconv"
)
func generate(ch chan<- int) {
	for i := 2; ; i++ {
		ch <- i
	}
}

func filter(in <-chan int, out chan<- int, prime int) {
	for i := range in {
		if i % prime != 0 {
			out <- i
		}
	}
}

func main() {
	n := func() int64 {
		tmp, _ := strconv.Atoi(os.Args[1])
		return int64(tmp)
	}()
	ch := make(chan int)
	var prime int
	start := time.Now()
	go generate(ch)
	for i := int64(0); i < n; i++ {
		prime = <-ch
		ch1 := make(chan int)
		go filter(ch, ch1, prime)
		ch = ch1
	}
	elapsed := time.Since(start).Nanoseconds()
	fmt.Println(n, ",", elapsed/n)
}
