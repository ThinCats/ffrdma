package utils

import (
	"bufio"
	"io"
)

// EWriter represents error Writer, wraps bufio.Writer
type EWriter struct {
	writer *bufio.Writer
	err    error
}

// NewEWriter creates a writer
func NewEWriter(w io.Writer) *EWriter {
	return &EWriter{
		writer: bufio.NewWriter(w),
		err:    nil,
	}
}

// WriteString writes string into io
func (w *EWriter) WriteString(s string) (int, error) {
	if w.err != nil {
		return 0, w.err
	}
	n, err := w.writer.WriteString(s)
	w.err = err
	return n, err
}

// Flush flush data to writer
func (w *EWriter) Flush() error {
	return w.writer.Flush()
}
