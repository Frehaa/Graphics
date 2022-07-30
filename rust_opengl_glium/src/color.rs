

#[derive(Copy, Clone, Debug)]
pub enum Color {
    RED, 
    GREEN,
    BLUE,
    YELLOW
}

impl Color {
    pub fn next_color(self) -> Color{
        match self {
            Color::RED => Color::GREEN,
            Color::GREEN => Color::BLUE,
            Color::BLUE => Color::YELLOW,
            Color::YELLOW => Color::RED,
        }
    }

    pub fn to_vec3(self) -> (f32, f32, f32) {
        match self {
            Color::RED => (0.6, 0.0, 0.0),
            Color::GREEN => (0.0, 0.6, 0.0),
            Color::BLUE => (0.0, 0.0, 0.6),
            Color::YELLOW => (0.6, 0.6, 0.0),
        }
    }
}