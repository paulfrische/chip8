#version 330

#define WARPX 0.05
#define WARPY 0.05
#define LINE_COUNT 200.0
#define TINT 0.95
#define VIGNETTE 0.5
#define BLURRES 0.0001

in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;

out vec4 finalColor;

vec4 sample(vec2 pos) {
  return texture(texture0, (pos - 1.0) / 2.0);
}

vec2 warp(vec2 pos) {
  return pos * vec2(1.0 + (pos.y * pos.y) * WARPX, 1.0 + (pos.x * pos.x) * WARPY);
}

vec4 blur(vec2 pos, float size) {
  int count = 0;
  vec4 acc = vec4(0.0);
  for (float x = -size; x < size; x += BLURRES) {
    for (float y = -size; y < size; y += BLURRES) {
      acc += sample(vec2(pos.x + x, pos.y + y));
      count++;
    }
  }

  return acc / count;
}

void main() {
  vec2 opos = fragTexCoord * 2.0 - 1.0;

  // warp
  vec2 pos = warp(opos);
  finalColor = sample(pos);

  // vignette
  finalColor *= mix(1.0, VIGNETTE, length(opos));

  // blur
  finalColor *= blur(pos, 0.001);

  // scanlines
  if (mod(floor(abs((pos.y + 1) / 2) * LINE_COUNT), 2.0) == 0) {
    finalColor *= TINT;
  }

  finalColor.a = 1.0;
}
