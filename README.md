# Código no simulador online
https://wokwi.com/projects/475908575158979585

# REQUER A EXTENSÃO WOKWI SIMULATOR
https://marketplace.visualstudio.com/items?itemName=Wokwi.wokwi-vscode

# REQUER O arduino-cli (compilar código)
## Baixando/instalando (instalará na pasta bin do diretório em que se executa o comando)
```curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | sh```
```bin/arduino-cli.exe core update-index```

## Instalando compilador pro ESP32
```bin/arduino-cli.exe core install esp32:esp32```

## Compilando projeto
```bin/arduino-cli compile --fqbn esp32:esp32:esp32 --output-dir build .```