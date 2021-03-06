#ifndef _COLORS_H
#define _COLORS_H

#define COLOR_TYPES 9
#define COLOR_SCHEME_SIZE COLOR_TYPES * 4
#define COLOR_SCHEME_COUNT 6
#define BG_OFFSET 0
#define OVERLAY_OFFSET 4
#define BORDER_OFFSET 8
#define FONT_OFFSET 12
#define COLORS_OFFSET 16

static const float COLOR_SCHEMES[COLOR_SCHEME_COUNT][COLOR_SCHEME_SIZE] = {
    // Dark: Green and gray/blue
    {
        0.015, 0.065, 0.215, 1.0,  // Background
        0.015, 0.065, 0.215, 0.7,  // Overlay
        1.000, 1.000, 1.000, 0.6,  // Border
        1.000, 1.000, 1.000, 1.0,  // Font
        0.031, 0.031, 0.031, 0.1,  // Dead
        1.000, 1.000, 1.000, 1.0,  // Born
        0.266, 0.266, 0.266, 0.8,  // Dying
        0.000, 0.733, 0.000, 1.0,  // Alive
        0.000, 0.733, 0.000, 1.0,  // Still Alive
    },

    // Dark: Green and gray/black
    {
        0.015, 0.015, 0.015, 1.0,  // Background
        0.266, 0.266, 0.266, 0.8,  // Overlay
        0.000, 0.733, 0.000, 1.0,  // Border
        1.000, 1.000, 1.000, 1.0,  // Font
        0.031, 0.031, 0.031, 1.0,  // Dead
        1.000, 1.000, 1.000, 1.0,  // Born
        0.266, 0.266, 0.266, 1.0,  // Dying
        0.000, 0.733, 0.000, 1.0,  // Alive
        0.000, 0.733, 0.000, 1.0,  // Still Alive
    },

    // Dark: Blue and gray/black
    {
        0.031, 0.031, 0.031, 1.0,  // Background
        0.227, 0.231, 0.247, 0.6,  // Dying
        0.016, 0.804, 1.000, 1.0,  // Border
        1.000, 1.000, 1.000, 1.0,  // Font
        0.000, 0.000, 0.000, 1.0,  // Dead
        1.000, 1.000, 1.000, 1.0,  // Born
        0.227, 0.231, 0.247, 1.0,  // Dying
        0.016, 0.804, 1.000, 1.0,  // Alive
        0.016, 0.804, 1.000, 1.0,  // Still Alive
    },

    // Light: Blue and gray/white
    {
        0.867, 0.867, 0.867, 1.0,  // Background
        0.867, 0.867, 0.867, 0.7,  // Overlay
        0.718, 0.941, 1.000, 1.0,  // Border
        0.000, 0.000, 0.000, 1.0,  // Font
        0.937, 0.937, 0.937, 1.0,  // Dead
        0.718, 0.941, 1.000, 1.0,  // Born
        0.667, 0.667, 0.667, 1.0,  // Dying
        0.016, 0.804, 1.000, 1.0,  // Alive
        0.016, 0.804, 1.000, 1.0,  // Stil Alive
    },

    // Dark: Red and white/black
    {
        0.000, 0.000, 0.000, 1.0,  // Background
        0.267, 0.267, 0.267, 0.8,  // Overlay
        0.933, 0.133, 0.133, 1.0,  // Border
        1.000, 1.000, 1.000, 1.0,  // Font
        0.016, 0.016, 0.016, 1.0,  // Dead
        0.267, 0.267, 0.267, 1.0,  // Born
        0.933, 0.133, 0.133, 1.0,  // Dying
        1.000, 1.000, 1.000, 1.0,  // Alive
        1.000, 1.000, 1.000, 1.0,  // Still Alive
    },

    // Dark: Blue and black/white
    {
        0.000, 0.000, 0.000, 1.0,  // Background
        0.633, 0.633, 0.633, 0.6,  // Overlay
        0.016, 0.804, 1.000, 1.0,  // Border
        1.000, 1.000, 1.000, 1.0,  // Font
        0.267, 0.267, 0.267, 1.0,  // Dead
        0.933, 0.933, 0.933, 1.0,  // Born
        0.133, 0.133, 0.133, 1.0,  // Dying
        0.016, 0.804, 1.000, 1.0,  // Alive
        0.016, 0.804, 1.000, 1.0,  // Still Alive
    },
};

#endif
/* vim: set ft=c : */
