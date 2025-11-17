

case "$1" in   
    start)
        echo "Starting lwip_app"
        start-stop-daemon -S -n lwip_app -x /usr/bin/lwip_app -- -d
        ;;
    stop)
        echo "Stopping lwip_app"
        start-stop-daemon -K -n lwip_app
        ;;
    *)
        echo "Usage: $0 {start|stop}"
    exit 1
esac

