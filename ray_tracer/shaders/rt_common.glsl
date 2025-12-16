struct HitPayload
{
    vec3 color;
    uint seed;
    vec3 rayOrigin;
    int  depth;
    vec3 rayDirection;
    int  done;
    vec3 weight;
};