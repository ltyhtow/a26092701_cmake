# xioTechnologies Fusion

The `fusion` directory contains the upstream C implementation from
`xioTechnologies/Fusion`.

- Upstream: https://github.com/xioTechnologies/Fusion
- Revision imported: `a8d7224f36a0ec82345ef49a3db50e65f8d3bab8`
- License: MIT (`fusion/LICENSE.md`)

The application builds the 6-axis components needed by this project:

- `FusionAhrs`: gyroscope plus accelerometer attitude estimation;
- `FusionBias`: run-time gyroscope offset estimation;
- `FusionRemap`: sensor-to-body axis mapping;
- `FusionModel.h`: calibration model helpers retained for later calibration data.

The upstream sources are kept unchanged. `src/imu_fusion.c` supplies the
project-specific startup stationary calibration, units, sample timing, and
configuration.
