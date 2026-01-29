#![no_main]

#[no_mangle]
pub extern "C" fn color_from_frame(frame: u32, out_rgba: *mut u8) {
    if out_rgba.is_null() {
        return;
    }

    let t = frame as f32 * 0.025;
    let r = (t.sin() * 127.0 + 128.0) as u8;
    let g = ((t + 2.0943952).sin() * 127.0 + 128.0) as u8;
    let b = ((t + 4.1887903).sin() * 127.0 + 128.0) as u8;

    unsafe {
        *out_rgba.add(0) = r;
        *out_rgba.add(1) = g;
        *out_rgba.add(2) = b;
        *out_rgba.add(3) = 255;
    }
}
