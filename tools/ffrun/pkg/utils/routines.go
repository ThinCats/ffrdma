package utils

import (
	"sync"
)

// ExecuteFunc represents func
type ExecuteFunc func()

// WaitAny waits any function executed, then end
func WaitAny(funcs ...ExecuteFunc) {
	closeCh := make(chan struct{})
	for _, fn := range funcs {
		go func(f ExecuteFunc) {
			f()
			select {
			case closeCh <- struct{}{}:
			default:
			}
		}(fn)
	}
	<-closeCh
}

// WaitAll wait all function to executed
func WaitAll(funcs ...ExecuteFunc) {
	var wg sync.WaitGroup
	for _, fn := range funcs {
		wg.Add(1)
		go func(f ExecuteFunc) {
			defer wg.Done()
			f()
		}(fn)
	}
	wg.Wait()
}
