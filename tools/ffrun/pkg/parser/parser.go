package parser

// parser for singularity command

// FormatType defines format
type FormatType struct {
	// Head is use like 'Program', but with space within it
	Head string
	// Tail is use like 'Program Args', usually treated as args
	Tail string
}

// Parser implements interface
type Parser interface {
	Parse([]string) (*FormatType, error)
}
