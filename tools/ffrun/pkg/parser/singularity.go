package parser

import (
	"errors"
	"strings"
)

// Singularity parser
// [sudo] singularity exec -fw --writable <image> <program> <program args>
// The parser will convernt args to these format
// [singularity exec -fw --writable <image> <program>] [<program args>]
type Singularity struct{}

func (s *Singularity) Parse(args []string) (*FormatType, error) {
	format := &FormatType{
		Head: "",
		Tail: "",
	}

	// at least 3 args singularity, exec, <program>
	if len(args) < 3 {
		return nil, errors.New("singularity's arg is too short")
	}

	i := 2
	// !! TODO: Not hardcoded
	if args[0] == "sudo" {
		i = i + 1
	}
	// ignore [sudo] <singularity> <exec>
	for ; i < len(args); i++ {
		// first argument not -s or --install-dir
		if args[i][0] != '-' {
			break
		}
	}

	// ignore <image> and <program>
	i += 2

	if i > len(args) {
		return nil, errors.New("singularity not sepcific image or program to run")
	}

	format.Head = strings.Join(args[:i], " ")
	format.Tail = strings.Join(args[i:], " ")
	return format, nil
}
