pub fn add_mut(a: &mut [f32; 3], b: &[f32; 3]) {
    for (i, j) in a.iter_mut().zip(b.iter()) {
        *i += j;
    }
}

pub fn rotate((x, y): (f32, f32), rad: f32) -> (f32, f32) {
    (x * rad.cos() - y * rad.sin(), y * rad.cos() + x * rad.sin())
}