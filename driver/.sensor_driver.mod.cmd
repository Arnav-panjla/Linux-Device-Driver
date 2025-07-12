savedcmd_sensor_driver.mod := printf '%s\n'   sensor_driver.o | awk '!x[$$0]++ { print("./"$$0) }' > sensor_driver.mod
