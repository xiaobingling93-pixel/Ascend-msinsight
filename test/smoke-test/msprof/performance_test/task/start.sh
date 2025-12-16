#!/bin/bash
set -e

script=$(readlink -f "$0")
route=$(dirname "$script")

function main() {
  server=${route}/resources/profiler/server/profiler_server
  frontend=${route}/resources/profiler/frontend
  os=$(uname -s)
  python="python3"

  if [ "${os:0:5}" == 'MINGW' ]; then
    server=${server}".exe"
    python="python"
  fi
  cd "${route}"
  
  export LD_LIBRARY_PATH="${route}/resources/profiler/server":$LD_LIBRARY_PATH
  
#   port=$("${server}" --scan=9000)
#   port=${port:16:5}
  
#   echo "Start server listen, port: ${port}"
  
  "${server}" --wsPort=9000 --wsHost=0.0.0.0 --logPath=/home/profiler_performance/task/Insight &
  serverPid=$!
  ${python} -m http.server -d "${frontend}" 8085 &
  webPid=$!
  wait
}

trap_ctrlc() {
  kill -9 ${serverPid} && kill -9 ${webPid}
}

trap trap_ctrlc INT

main