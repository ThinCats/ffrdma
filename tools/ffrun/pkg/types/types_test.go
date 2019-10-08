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

func TestHostMapPorts_FromString(t *testing.T) {
	type args struct {
		str string
	}
	tests := []struct {
		name    string
		h       HostMapPorts
		args    args
		want    HostMapPorts
		wantErr bool
	}{
		// TODO: Add test cases.
		{
			name: "Test 1",
			h:    nil,
			args: args{
				"node110:100+200+300,node112:500+400+300",
			},
			want: HostMapPorts{
				"node110": []int{100, 200, 300},
				"node112": []int{500, 400, 300},
			},
			wantErr: false,
		},
	}
	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			got, err := tt.h.FromString(tt.args.str)
			if (err != nil) != tt.wantErr {
				t.Errorf("HostMapPorts.FromString() error = %v, wantErr %v", err, tt.wantErr)
				return
			}
			if !reflect.DeepEqual(got, tt.want) {
				t.Errorf("HostMapPorts.FromString() = %v, want %v", got, tt.want)
			}
		})
	}
}

func TestHostMapNumproc_FromString(t *testing.T) {
	type args struct {
		str string
	}
	tests := []struct {
		name    string
		h       HostMapNumproc
		args    args
		want    HostMapNumproc
		wantErr bool
	}{
		// TODO: Add test cases.
	}
	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			got, err := tt.h.FromString(tt.args.str)
			if (err != nil) != tt.wantErr {
				t.Errorf("HostMapNumproc.FromString() error = %v, wantErr %v", err, tt.wantErr)
				return
			}
			if !reflect.DeepEqual(got, tt.want) {
				t.Errorf("HostMapNumproc.FromString() = %v, want %v", got, tt.want)
			}
		})
	}
}
