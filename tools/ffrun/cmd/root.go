package cmd

import "github.com/mkideal/cli"

// Help is help command
var Help = cli.HelpCommand("Help")

// root command
type rootT struct {
	cli.Helper
}

// Root is root command
var Root = &cli.Command{
	Desc: "FFRUN runs program with library ffrdma",
	Argv: func() interface{} { return new(rootT) },
	Fn: func(ctx *cli.Context) error {
		ctx.String(ctx.Usage())
		return nil
	},
}
