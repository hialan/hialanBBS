ps -ax | grep mbbsd | awk '{print $1}' | xargs kill -9
echo "All mbbsd processes have been killed."

