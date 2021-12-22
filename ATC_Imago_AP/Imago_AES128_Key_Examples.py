#Imagotag AES Key preperation for Display Communications

import hashlib
from base64 import b64decode
from base64 import b64encode
from Crypto.Cipher import AES

def strToByte(inString):
    return bytes.fromhex(inString.upper())
    
def byteToStr(inBytes):
    return ''.join('{:02x}'.format(x) for x in inBytes).upper()
    
def decryptAES(keyAES, ivAES, dataAES):
    cipher = AES.new(strToByte(keyAES), AES.MODE_CBC, strToByte(ivAES))
    return byteToStr(cipher.decrypt(strToByte(dataAES)))

def encryptAES(keyAES, ivAES, dataAES):
    cipher = AES.new(strToByte(keyAES), AES.MODE_CBC, strToByte(ivAES))
    return byteToStr(cipher.encrypt(strToByte(dataAES)))

def getDisplayKey(systemKey, displaySerial):
    h = hashlib.sha256()
    h.update(systemKey.encode('utf-8'))
    base64digest = b64encode(strToByte(h.hexdigest()[0:32])).decode('utf-8')
    endHash = hashlib.sha256()
    endHash.update(base64digest.encode('utf-8')+displaySerial.upper().encode('utf-8'))
    return endHash.hexdigest()[0:32]

#System wide AES128 CBC Key
system_aes_key = "090e9dd1a6735b2a97a0f28856be99fa"

#current display serial
display_serial = "C400D3BC"

print("")
print("")

# Display Key generation part:###########################################
display_aes_key = getDisplayKey(system_aes_key, display_serial)
print("Demo Display key for display: " + display_serial + " is: " + display_aes_key)


print("")
print("")

# Actual Encryption part:################################################
display_aes_key = "5c2186b33cb80d61ae290cdc0f22b5da"
#The data that needs to be send to display
new_data_for_display = "6e7a34d7c8b86244518c1fb0897ab69500000000000000000000000000000000"

#Display first radnom iv in
random_iv_display = "2A1D8A25D8DAD999A97DA27BE28B26D8"

#AP to display data
random_iv_ap = "602B9D692E9D295EB972E4C8158FE6CB"
calculated_IV_TX = ""#will be calculated on the fly should be "8E50C0A4F66565946338AEFE6509279D"

#Display response
calculated_IV_RX = ""#will be calculated on the fly should be "4841AEC1ED00E29F0814788D54694786"
random_from_display = "080583A3FB0140F387209B696E6D2D9D"

print("Input data")
print("Key:     "+display_aes_key)
print("IV:      "+random_iv_ap)
print("DATA:    "+random_iv_display)
print("")


calculated_IV_TX = encryptAES(display_aes_key, random_iv_display, random_iv_ap)
print("calculated_IV_TX: "+ calculated_IV_TX)
print("")

calculated_IV_RX = encryptAES(display_aes_key, random_from_display, random_iv_ap)
print("calculated_IV_RX: "+ calculated_IV_RX)
print("")

decData1 = encryptAES(display_aes_key, calculated_IV_TX, new_data_for_display)
print("Encrypted real data: "+ decData1)
print("")