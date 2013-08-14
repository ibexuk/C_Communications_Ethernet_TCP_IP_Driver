PLEASE NOTE
An embedded-code customer has kindly provided this NIC driver following adapting our driver for the Stellaris LM3S6965.
Embedded-code.com can't provide support or waranties for third party code but we provide with permission it in case it may prove useful to other users.


Notes from the author:-

I send the driver for Stellaris LM3S6965 nic
I did several tests I hope it is ok
I had to define a buffer RAM because you can not move back the pointer of the nic buffer (as could nic_move_pointer)
I tested the driver with Rowley and IAR compilers

Last thing file eth_ip.c
function: void ip_add_bytes_to_ip_checksum (WORD *checksum, BYTE *checksum_next_byte_is_low, BYTE *next_byte, BYTE no_of_bytes_to_add)
the last parameter no_of_bytes_to_add I think is WORD not BYTE (I have checksum error with TFTP protocol)