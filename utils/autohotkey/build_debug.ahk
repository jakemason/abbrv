#SingleInstance

; Go up one directory
SetWorkingDir ".\.."

F3::{
    Run A_WorkingDir "\build_debug.cmd"
}
