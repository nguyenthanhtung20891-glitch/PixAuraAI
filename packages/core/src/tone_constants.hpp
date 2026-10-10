#ifndef PIXAURA_TONE_CONSTANTS_HPP
#define PIXAURA_TONE_CONSTANTS_HPP
namespace pixaura::tone {
inline constexpr double rgb_xyz[3][3] = {
    {0x1.a649c610130b1p-2, 0x1.6e2a96ccdcb17p-2, 0x1.719fe95deff92p-3},
    {0x1.b37c144093a36p-3, 0x1.6e2a96ccdcb17p-1, 0x1.27b32117f32dbp-4},
    {0x1.3cb7548c0e484p-6, 0x1.e838c9112641fp-4, 0x1.e6ac26776ae5ep-1},
};
inline constexpr double xyz_rgb[3][3] = {
    {0x1.9ed81a61e643ap+1, -0x1.8991f1a515886p+0, -0x1.fe93d1b3918e5p-2},
    {-0x1.f040b3af5d706p-1, 0x1.e03f67fb55a13p+0, 0x1.546b459182d75p-5},
    {0x1.c7b8bb9f1e66dp-5, -0x1.a1beabfc8688fp-3, 0x1.0e95af667a0d1p+0},
};
inline constexpr double bradford[3][3] = {
    {0x1.ca4a8c154c986p-1, 0x1.10cb295e9e1b1p-2, -0x1.4a8c154c985f0p-3},
    {-0x1.801a36e2eb1c4p-1, 0x1.b6a7ef9db22d1p+0, 0x1.2ca57a786c227p-5},
    {0x1.3eab367a0f909p-5, -0x1.189374bc6a7f0p-4, 0x1.0793dd97f62b7p+0},
};
inline constexpr double bradford_inverse[3][3] = {
    {0x1.f9572254ba230p-1, -0x1.2d2ac83086a83p-3, 0x1.479a7fabd13c4p-3},
    {0x1.baae3b8d66d95p-2, 0x1.0966847b979f9p-1, 0x1.93cb32a4eca54p-5},
    {-0x1.17779fb6e784ap-7, 0x1.4807e22e340fap-5, 0x1.efdd7cfa0989ap-1},
};
}
#endif
