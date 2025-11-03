# ANSII static generator(ansii-readable)

This software generates ANSII from a human readable syntax, for the aim of
using this to redirect `curl` requests to terminal-readable and colorful
text.

Syntax example:(retains spaces)

```
{{bg:red}} simple color syntax {{bg:default}}
{{bg:200-200-200}}<< RGB colored >>{{bg:black}}
{{bg:id-142}} with ANSII ids...{{bg:black}}
{{bg:id-142}} with ANSII ids...{{bg:black}}

