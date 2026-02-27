case "$1" in
    start)
        printf "Starting MyApp: "
        /usr/bin/voice_cap &
        echo "OK"
        ;;
    stop)
        printf "Stopping MyApp: "
        killall voice_cap
        echo "OK"
        ;;
    restart)
        $0 stop
        $0 start
        ;;
    *)
        echo "Usage: $0 {start|stop|restart}"
        exit 1
        ;;
esac
exit 0

