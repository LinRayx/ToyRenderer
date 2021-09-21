#version 450


void main()
{
    gl_FragDepth = gl_FragCoord.z * 0.5 + 0.5;
}