package utils

import (
	"os"
	"os/signal"
	"syscall"
)

// NotifySignals will create a channel on specific signals
func NotifySignals(singals ...os.Signal) chan os.Signal {
	sigs := make(chan os.Signal, 1)
	signal.Notify(sigs, singals...)
	return sigs
}

// NotifySignalsDefault notify default signals
func NotifySignalsDefault() chan os.Signal {
	return NotifySignals(syscall.SIGINT, syscall.SIGTERM)
}
