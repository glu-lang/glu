/// Update position/velocity with simple edge bouncing.
export fn updateMotion(
    pos: [*]f32,
    vel: [*]f32,
    bounds: [*]const f32,
    radius: f32,
    dt: f32,
) void {
    var x = pos[0];
    var y = pos[1];
    var vx = vel[0];
    var vy = vel[1];

    x += vx * dt;
    y += vy * dt;

    const max_x = bounds[0] - radius * 2.0;
    const max_y = bounds[1] - radius * 2.0;

    if (x <= 0.0) {
        x = 0.0;
        vx = -vx;
    } else if (x >= max_x) {
        x = max_x;
        vx = -vx;
    }

    if (y <= 0.0) {
        y = 0.0;
        vy = -vy;
    } else if (y >= max_y) {
        y = max_y;
        vy = -vy;
    }

    pos[0] = x;
    pos[1] = y;
    vel[0] = vx;
    vel[1] = vy;
}
