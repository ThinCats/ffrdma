package types

import (
	"reflect"
	"sort"
	"testing"
)

func Test_sort(t *testing.T) {
	type args struct {
		s ProcessInfoSlice
	}
	tests := []struct {
		name string
		args args
		want ProcessInfoSlice
	}{
		{
			name: "simple test",
			args: args{
				s: ProcessInfoSlice{
					{Host: "10.10.0.110", Port: 1000},
					{Host: "10.10.0.111", Port: 1002},
					{Host: "10.10.0.110", Port: 1003},
					{Host: "10.10.0.110", Port: 1007},
					{Host: "10.10.0.112", Port: 1003},
					{Host: "10.10.0.110", Port: 1001},
					{Host: "10.10.0.111", Port: 1001},
					{Host: "10.10.0.112", Port: 1001},
				},
			},
			want: ProcessInfoSlice{
				{Host: "10.10.0.110", Port: 1000},
				{Host: "10.10.0.110", Port: 1001},
				{Host: "10.10.0.110", Port: 1003},
				{Host: "10.10.0.110", Port: 1007},
				{Host: "10.10.0.111", Port: 1001},
				{Host: "10.10.0.111", Port: 1002},
				{Host: "10.10.0.112", Port: 1001},
				{Host: "10.10.0.112", Port: 1003}},
		},
	}
	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			sort.Sort(tt.args.s)
			got := tt.args.s
			if !reflect.DeepEqual(got, tt.want) {
				t.Errorf("sort() = %v, want %v", got, tt.want)
			}
		})
	}
}

func TestHostMapPorts_String(t *testing.T) {
	tests := []struct {
		name string
		h    HostMapPorts
		want string
	}{
		{
			name: "Test 1",
			h: HostMapPorts{
				"10.10.0.1": []int{1, 2, 3, 4},
				"10.10.0.2": []int{100, 500},
			},
			want: "10.10.0.1:1+2+3+4,10.10.0.2:100+500",
		},
	}
	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			if got := tt.h.String(); got != tt.want {
				t.Errorf("HostMapPorts.String() = %v, want %v", got, tt.want)
			}
		})
	}
}
