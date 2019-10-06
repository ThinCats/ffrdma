package main

import (
	"ffrun/cmd"
	"fmt"
	"os"

	"github.com/mkideal/cli"
)

func main() {
	if err := cli.Root(cmd.Root,
		cli.Tree(cmd.Help),
		cli.Tree(cmd.Generate),
	).Run(os.Args[1:]); err != nil {
		fmt.Fprintln(os.Stderr, err)
		os.Exit(1)
	}
}
