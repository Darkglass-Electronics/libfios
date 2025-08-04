{
  "targets": [
    {
      "target_name": "libfios",
      "sources": [
        "src/addon.cpp",
        "src/libfios-file.c",
        "src/libfios-serial.c",
        "src/libfios-export.c"
      ],
      "include_dirs": [
        "<!@(node -p \"require('node-addon-api').include\")"
      ],
      "defines": [ "NAPI_DISABLE_CPP_EXCEPTIONS" ]
    }
  ]
}
