#pragma once

#include <array>

namespace morpher
{
    inline constexpr int numSlots = 4;

    /** The four corners of the pad, in reading order. */
    enum Corner { cornerA = 0, cornerB = 1, cornerC = 2, cornerD = 3 };

    inline const char* cornerName (int slot)
    {
        static constexpr const char* names[] { "A", "B", "C", "D" };
        return (slot >= 0 && slot < numSlots) ? names[slot] : "?";
    }

    /** How much of each corner is heard at a point on the pad.

        x runs 0 (left) to 1 (right) and y runs 0 (bottom) to 1 (top), so
        A is top left, B top right, C bottom left and D bottom right.

        The weights are bilinear and always add up to 1: the sounds are mixed
        at their own level, which keeps a level steady when the corners sound
        alike - and plug-ins fed the same input usually do (decision 0003).
    */
    inline std::array<float, numSlots> weightsAt (float x, float y) noexcept
    {
        x = x < 0.0f ? 0.0f : (x > 1.0f ? 1.0f : x);
        y = y < 0.0f ? 0.0f : (y > 1.0f ? 1.0f : y);

        return { (1.0f - x) * y,             // A, top left
                 x * y,                      // B, top right
                 (1.0f - x) * (1.0f - y),    // C, bottom left
                 x * (1.0f - y) };           // D, bottom right
    }

    /** Where each corner sits, as an (x, y) pair. */
    inline std::array<float, 2> cornerPosition (int slot) noexcept
    {
        switch (slot)
        {
            case cornerA: return { 0.0f, 1.0f };
            case cornerB: return { 1.0f, 1.0f };
            case cornerC: return { 0.0f, 0.0f };
            default:      return { 1.0f, 0.0f };
        }
    }
}
