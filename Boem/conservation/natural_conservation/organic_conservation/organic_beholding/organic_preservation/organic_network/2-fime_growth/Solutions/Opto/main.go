package main

import (
	"encoding/json"
	"fmt"
	"net/http"
	"os"
	"os/exec"
	"runtime"
)

type Task struct {
	Category string `json:"category"`
	Title    string `json:"title"`
	Done     bool   `json:"done"`
}

var tasks []Task
var stateFile = os.Getenv("HOME") + "/Documents/opto_state.json"

func saveState() {
	data, _ := json.MarshalIndent(tasks, "", "  ")
	_ = os.WriteFile(stateFile, data, 0644)
}

func loadState() {
	data, err := os.ReadFile(stateFile)
	if err == nil {
		_ = json.Unmarshal(data, &tasks)
	}
}

func main() {
	loadState()

	http.Handle("/", http.FileServer(http.Dir("static")))

	http.HandleFunc("/api/tasks", func(w http.ResponseWriter, r *http.Request) {
		if r.Method == "GET" {
			json.NewEncoder(w).Encode(tasks)
		} else if r.Method == "POST" {
			var t Task
			_ = json.NewDecoder(r.Body).Decode(&t)
			tasks = append(tasks, t)
			saveState()
			w.WriteHeader(http.StatusCreated)
		}
	})

	http.HandleFunc("/api/toggle", func(w http.ResponseWriter, r *http.Request) {
		var req struct{ Index int }
		_ = json.NewDecoder(r.Body).Decode(&req)
		if req.Index >= 0 && req.Index < len(tasks) {
			tasks[req.Index].Done = !tasks[req.Index].Done
			saveState()
		}
	})

	fmt.Println("OPTO running at http://localhost:8080")
	go openBrowser("http://localhost:8080")
	_ = http.ListenAndServe(":8080", nil)
}

func openBrowser(url string) {
	var cmd string
	var args []string

	switch runtime.GOOS {
	case "darwin":
		cmd = "open"
	case "windows":
		cmd = "rundll32"
		args = []string{"url.dll,FileProtocolHandler"}
	default:
		cmd = "xdg-open"
	}
	args = append(args, url)
	_ = exec.Command(cmd, args...).Start()
}
