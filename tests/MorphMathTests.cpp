#include <juce_core/juce_core.h>
#include "MorphMath.h"

using namespace morpher;

struct MorphMathTests final : juce::UnitTest
{
    MorphMathTests() : juce::UnitTest ("The morph's weights", "Morpher") {}

    void runTest() override
    {
        beginTest ("Each corner is that plug-in alone");
        for (int s = 0; s < numSlots; ++s)
        {
            const auto p = cornerPosition (s);
            const auto w = weightsAt (p[0], p[1]);

            for (int t = 0; t < numSlots; ++t)
                expectWithinAbsoluteError (w[(size_t) t], t == s ? 1.0f : 0.0f, 1.0e-6f);
        }

        beginTest ("A is top left, B top right, C bottom left, D bottom right");
        expectEquals (weightsAt (0.0f, 1.0f)[cornerA], 1.0f);
        expectEquals (weightsAt (1.0f, 1.0f)[cornerB], 1.0f);
        expectEquals (weightsAt (0.0f, 0.0f)[cornerC], 1.0f);
        expectEquals (weightsAt (1.0f, 0.0f)[cornerD], 1.0f);

        beginTest ("The centre is a quarter of each");
        for (auto w : weightsAt (0.5f, 0.5f))
            expectWithinAbsoluteError (w, 0.25f, 1.0e-6f);

        beginTest ("The weights always add up to 1 and are never negative");
        for (int i = 0; i <= 40; ++i)
        {
            for (int j = 0; j <= 40; ++j)
            {
                const auto w = weightsAt ((float) i / 40.0f, (float) j / 40.0f);
                float sum = 0.0f;

                for (auto v : w)
                {
                    expect (v >= 0.0f);
                    sum += v;
                }

                expectWithinAbsoluteError (sum, 1.0f, 1.0e-5f);
            }
        }

        beginTest ("Moving toward a corner never makes it quieter");
        for (int i = 0; i < 20; ++i)
        {
            const auto t0 = (float) i / 20.0f, t1 = (float) (i + 1) / 20.0f;
            expect (weightsAt (t1, 0.5f)[cornerB] >= weightsAt (t0, 0.5f)[cornerB]);
            expect (weightsAt (0.5f, t1)[cornerA] >= weightsAt (0.5f, t0)[cornerA]);
        }

        beginTest ("Out of range is held at the edge");
        expectEquals (weightsAt (-1.0f, 2.0f)[cornerA], 1.0f);
        expectEquals (weightsAt (5.0f, -3.0f)[cornerD], 1.0f);
    }
};

static MorphMathTests morphMathTests;
