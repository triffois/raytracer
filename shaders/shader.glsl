#version 430 core
out vec4 FragColor;
in vec4 gl_FragCoord;

uniform vec2 iResolution;
uniform float iTime;
uniform int iFrame;
uniform int triangle_count;
// AABB
uniform float max_x;
uniform float min_x;
uniform float max_y;
uniform float min_y;
uniform float max_z;
uniform float min_z;
uniform int root_id;

struct Triangle {
    vec4 v1;
    vec4 v2;
    vec4 v3;
    vec4 min;
    vec4 max;
};

struct Box {
    vec4 min;
    vec4 max;
    int left_id;
    int right_id;
    int start;
    int end;
};

layout(std430, binding = 3) buffer triangles_ssbo {
    Triangle triangles[];
};

layout(std430, binding = 4) buffer boxes_ssbo {
    Box boxes[];
};

struct Sphere {
    vec3 center;
    float radius;
};

struct Ray {
    vec3 origin;
    vec3 direction;
};

vec3 hitsSphere(Ray ray, Sphere sphere) {
    vec3 x_p = ray.origin - sphere.center;
    float a = dot(ray.direction, ray.direction);
    if (a == 0.0) {
        return vec3(1.0, 1.0, 1.0);
    }
    float b = 2.0 * dot(ray.direction, x_p);
    float c = dot(x_p, x_p) - sphere.radius * sphere.radius;
    float discriminant = b * b - 4.0 * a * c;
    if (discriminant < 0.0) {
        return vec3(1.0, 1.0, 1.0);
    }
    float k = (-b - sqrt(discriminant)) / (2.0 * a);
    if (k < 0.0) {
        return vec3(1.0, 1.0, 1.0);
    }
    return ray.origin + k * ray.direction;
};

vec3 hitsGround(Ray ray, float height) {
    if (ray.direction.y == 0.0) {
        return vec3(1.0, 1.0, 1.0);
    }
    float k = (height - ray.origin.y) / ray.direction.y;
    if (k < 0.0) {
        return vec3(1.0, 1.0, 1.0);
    }
    return ray.origin + k * ray.direction;
}

float getBrightnessToPos(vec3 position, vec3 normal, vec3 light_position,
    float sourceBrightness) {
    vec3 vecFromLight = position - light_position;
    float lengthFromLight = length(vecFromLight);
    return sourceBrightness / lengthFromLight / lengthFromLight *
        max(dot(normal, normalize(vecFromLight)), 0);
}

void main() {
    vec3 origin = vec3(0.0);
    float focalLength = 1.0;
    vec2 uv = gl_FragCoord.xy / iResolution - 0.5;
    float size_y = 1.0 / iResolution.x * iResolution.y;
    vec3 screenPos = vec3(uv.x, uv.y * size_y, focalLength);

    Ray ray = Ray(origin, normalize(screenPos));

    Sphere sphere = Sphere(vec3(sin(iTime) * focalLength * 1.5,
                cos(iTime) * focalLength * 1.5, 8 * focalLength),
            focalLength);
    vec3 hitSphere = hitsSphere(ray, sphere);

    vec3 hitGround = hitsGround(ray, -focalLength);

    vec3 inherentSphereColor = vec3(0.6, 0.7, 0.3);
    vec3 inherentGroundColor = vec3(0.3, 0.3, 0.3);
    float rotationRadius = 6.0;
    float rotationSpeed = 0.2;
    vec3 lightPosition = vec3(1 + rotationRadius * sin(iTime * rotationSpeed), 5,
            4 + rotationRadius * cos(iTime * rotationSpeed));

    float brightness = 10;
    if (hitSphere == vec3(1.0, 1.0, 1.0) && hitGround == vec3(1.0, 1.0, 1.0)) {
        FragColor = vec4(0.0, 0.0, 0.0, 1.0);
        return;
    }
    vec3 color;
    if (hitSphere == vec3(1) ||
            (hitGround != vec3(1) &&
                length(hitSphere - origin) > length(hitGround - origin))) {
        vec3 hitSphereFromGroundToLight = hitsSphere(
                Ray(hitGround, normalize(lightPosition - hitGround)), sphere);
        if (hitSphereFromGroundToLight != vec3(1.0, 1.0, 1.0)) {
            color = vec3(0.0, 0.0, 0.0);
        } else {
            vec3 normal = vec3(0.0, -1.0, 0.0);
            color = inherentGroundColor *
                    getBrightnessToPos(hitGround, normal, lightPosition, brightness);
        }
    } else {
        vec3 normal = normalize(-hitSphere + sphere.center);
        color = inherentSphereColor *
                getBrightnessToPos(hitSphere, normal, lightPosition, brightness);
    }
    FragColor = vec4(pow(color + vec3(0.005), vec3(1.0) / 2.2), 1.0);
}
