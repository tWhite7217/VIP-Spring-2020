import time
import magichue

light = magichue.Light('192.168.1.200', confirm_receive_on_send=False, allow_fading=False)

light.on = False