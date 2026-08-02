Import("env")

#
# ESP Prayer System
# Auto Upload Firmware + LittleFS
#

def after_upload(source, target, env):

    print("")
    print("==============================")
    print(" Uploading LittleFS Filesystem ")
    print("==============================")
    print("")

    env.Execute(
        "pio run -t uploadfs"
    )


#
# Run after firmware upload
#

env.AddPostAction(
    "$BUILD_DIR/${PROGNAME}.bin",
    after_upload
)