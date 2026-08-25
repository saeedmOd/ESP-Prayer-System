Import("env")

#
# ESP Prayer System
# Auto Upload Firmware + LittleFS
#
# DISABLED: uploadfs erases config.json (WiFi creds).
# Run `pio run -t uploadfs` manually when needed.
#

# def after_upload(source, target, env):
#
#     print("")
#     print("==============================")
#     print(" Uploading LittleFS Filesystem ")
#     print("==============================")
#     print("")
#
#     env.Execute(
#         "pio run -t uploadfs"
#     )
#
#
# env.AddPostAction(
#     "$BUILD_DIR/${PROGNAME}.bin",
#     after_upload
# )
