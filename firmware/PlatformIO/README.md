# PIO project

### Prerequisites:
```
pio update
```

### Update over wire:
```
pio run -e debug -t erase
pio run -e debug -t upload
pio run -e release -t upload
```

### Listen to serial monitor:
```
pio device monitor
```

### Update over local network:
```
pio run -e wireless -t upload
```
