Import("env")
env.AddTarget(
    "uploadfs",
    "$UPLOADCMD",
    "Upload filesystem image",
    "platform")