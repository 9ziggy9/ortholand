#version 330 core
out vec4 finalColor;

uniform vec4 lineColor;

void main() {
    float alpha =
      1.0 - smoothstep(0.0, 1.0, abs(gl_FragCoord.x - round(gl_FragCoord.x)));
    alpha *=
      1.0 - smoothstep(0.0, 1.0, abs(gl_FragCoord.y - round(gl_FragCoord.y)));
    finalColor = vec4(lineColor.rgb, alpha * lineColor.a);
}