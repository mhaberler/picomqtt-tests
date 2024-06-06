# devkit-m1
  3330][V][esp32-hal-periman.c:235] perimanSetBusDeinit(): Deinit function for type I2C_MASTER_SDA (30) successfully set to 0x42035448
[  3342][V][esp32-hal-periman.c:235] perimanSetBusDeinit(): Deinit function for type I2C_MASTER_SCL (31) successfully set to 0x42035448
[  3354][I][esp32-hal-i2c.c:109] i2cInit(): Initializing I2C Master: sda=8 scl=9 freq=100000
[  3362][V][esp32-hal-periman.c:160] perimanSetPinBus(): Pin 8 successfully set to type I2C_MASTER_SDA (30) with bus 0x1
[  3373][V][esp32-hal-periman.c:160] perimanSetPinBus(): Pin 9 successfully set to type I2C_MASTER_SCL (31) with bus 0x1
[  3383][V][esp32-hal-periman.c:235] perimanSetBusDeinit(): Deinit function for type I2C_MASTER_SDA (30) successfully set to 0x42035448
[  3395][V][esp32-hal-periman.c:235] perimanSetBusDeinit(): Deinit function for type I2C_MASTER_SCL (31) successfully set to 0x42035448
[  3407][I][esp32-hal-i2c.c:109] i2cInit(): Initializing I2C Master: sda=7 scl=6 freq=100000
[  3415][V][esp32-hal-periman.c:160] perimanSetPinBus(): Pin 7 successfully set to type I2C_MASTER_SDA (30) with bus 0x2
[  3426][V][esp32-hal-periman.c:160] perimanSetPinBus(): Pin 6 successfully set to type I2C_MASTER_SCL (31) with bus 0x2
[  9858][I][esp32-hal-periman.c:141] perimanSetPinBus(): Pin 19 already has type USB_DM (45) with bus 0x3fc9aa34
[  9859][I][esp32-hal-periman.c:141] perimanSetPinBus(): Pin 20 already has type USB_DP (46) with bus 0x3fc9aa34
[ 10457][V][esp32-hal-periman.c:235] perimanSetBusDeinit(): Deinit function for type SPI_MASTER_SCK (34) successfully set to 0x42037764
[ 10458][V][esp32-hal-periman.c:235] perimanSetBusDeinit(): Deinit function for type SPI_MASTER_MISO (35) successfully set to 0x4203768c
[ 10470][V][esp32-hal-periman.c:235] perimanSetBusDeinit(): Deinit function for type SPI_MASTER_MOSI (36) successfully set to 0x420375b4
[ 10481][V][esp32-hal-periman.c:235] perimanSetBusDeinit(): Deinit function for type SPI_MASTER_SS (37) successfully set to 0x4203749c
[ 10493][V][esp32-hal-periman.c:235] perimanSetBusDeinit(): Deinit function for type GPIO (1) successfully set to 0x420d03d4
[ 10504][V][esp32-hal-periman.c:160] perimanSetPinBus(): Pin 12 successfully set to type GPIO (1) with bus 0xd
[ 10514][V][esp32-hal-periman.c:160] perimanSetPinBus(): Pin 12 successfully set to type SPI_MASTER_SCK (34) with bus 0x1
[ 10525][V][esp32-hal-periman.c:235] perimanSetBusDeinit(): Deinit function for type GPIO (1) successfully set to 0x420d03d4
[ 10536][V][esp32-hal-periman.c:160] perimanSetPinBus(): Pin 13 successfully set to type GPIO (1) with bus 0xe
[ 10545][V][esp32-hal-periman.c:160] perimanSetPinBus(): Pin 13 successfully set to type SPI_MASTER_MISO (35) with bus 0x1
[ 10556][V][esp32-hal-periman.c:235] perimanSetBusDeinit(): Deinit function for type GPIO (1) successfully set to 0x420d03d4
[ 10567][V][esp32-hal-periman.c:160] perimanSetPinBus(): Pin 11 successfully set to type GPIO (1) with bus 0xc
[ 10577][V][esp32-hal-periman.c:160] perimanSetPinBus(): Pin 11 successfully set to type SPI_MASTER_MOSI (36) with bus 0x1
[ 10587][V][esp32-hal-periman.c:235] perimanSetBusDeinit(): Deinit function for type GPIO (1) successfully set to 0x420d03d4
[ 10598][V][esp32-hal-periman.c:160] perimanSetPinBus(): Pin 4 successfully set to type GPIO (1) with bus 0x5
[ 10608][V][esp32-hal-periman.c:174] perimanSetPinBusExtraType(): Successfully set extra_type SD_SS for pin 4
wire  sda=8  laport 0 schwarz
wire  scl=9  laport 0 braun

wire1  sda=7  laport 0 rot
wire1  scl=6  laport 0 orange

	-DDPS0_IRQ_PIN=0
	-DDPS1_IRQ_PIN=1
	-DDPS2_IRQ_PIN=14
	-DUBLOX_IRQ_PIN=4	
	-DUBLOX_PPS_PIN=5
	-DIMU_IRQ_PIN=2