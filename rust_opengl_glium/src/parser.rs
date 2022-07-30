use std::num::ParseFloatError;
use std::ops::Sub;
use std::path::Path;
use std::io::{ BufRead, BufReader};
use std::fs;
use glium::{ Display, VertexBuffer, IndexBuffer, vertex };
use glium::index::{PrimitiveType, Index};

use crate::primitive_shapes::{Vertex, Point};

struct Face { 
    v1 : usize,
    v2 : usize,
    v3 : usize,
    vn1 : usize,
    vn2 : usize,
    vn3 : usize
}

struct ObjFile {
    vertices:Vec<(f32, f32, f32)>,
    normals: Vec<(f32, f32, f32)>,
    faces: Vec<Face>
}

impl ObjFile {
    fn to_vertices(&self) -> Vec<Vertex> {
        let mut result = Vec::new();
        for Face { v1, v2, v3, vn1, vn2, vn3} in &self.faces {
            result.push(Vertex { position: self.vertices[v1-1], normal: self.normals[vn1-1]});
            result.push(Vertex { position: self.vertices[v2-1], normal: self.normals[vn2-1]});
            result.push(Vertex { position: self.vertices[v3-1], normal: self.normals[vn3-1]});
        }
        result
    }
}

fn parse_obj_coord(line: Vec<&str>) -> (f32, f32, f32){
    let d = line.iter()
                .skip(1)
                .map(|x| x.parse::<f32>() )
                .collect::<Result<Vec<_>,_>>()
                .unwrap();
    (d[0], d[1], d[2])
}

fn parse_obj_face(line: Vec<&str>) -> Face {
    let splits : Vec<Vec<_>> = 
        line.iter()
            .skip(1)
            .map(|part| {
                part.split('/')
                    .map(|x| { x.parse() })
                    .collect::<Result<Vec<_>,_>>()
                    .unwrap()
            })
            .collect();
    Face { v1: splits[0][0], v2: splits[1][0], v3: splits[2][0], vn1: splits[0][2], vn2: splits[1][2], vn3: splits[2][2] }
}

pub fn read_obj(filepath: &Path, display : &Display) -> (VertexBuffer<Vertex>, IndexBuffer<u16>){
    let file = fs::File::open(filepath).unwrap();
    let buf = BufReader::new(file);
    
    let mut obj = ObjFile {
        vertices: Vec::new(),
        normals: Vec::new(),
        faces: Vec::new(),
    };

    for line in buf.lines() {
        let line = line.unwrap();
        let split : Vec<&str> = line.split(' ').collect();
        match split[0] {
            "v" => obj.vertices.push(parse_obj_coord(split)),
            "vn"=> obj.normals.push(parse_obj_coord(split)),
            "f" => obj.faces.push(parse_obj_face(split)),
            _ => continue
        }
    }
    
    let position = &obj.to_vertices();
    let position = VertexBuffer::new(display, &position).unwrap();
    let indices = IndexBuffer::new(display, PrimitiveType::TrianglesList, &[0]).unwrap();
    // let indices = IndexBuffer::new(display, PrimitiveType::TrianglesList, &indices).unwrap();
    (position, indices)
}


// bez2 . 20 4
//      [3 0]
//      [3 1]
//      [1 1]        
//      [1 0]

#[derive(Debug)]
struct Bez {
    steps: usize,
    points: Vec<Point>
}

struct Bsp {
    steps: usize,
    points: Vec<Point>
}

use ndarray::{arr1, iter};
use ndarray::Array1;
use ndarray::Array;
use ndarray::Array0;

fn sub(lhs: &[f32; 3], rhs: &[f32; 3]) -> [f32; 3] {
    [
        lhs[0] - rhs[0],
        lhs[1] - rhs[1],
        lhs[2] - rhs[2],
    ]
}

fn scale(scalar: f32, v: &[f32; 3]) -> [f32; 3] {
    [
        v[0] * scalar,
        v[1] * scalar,
        v[2] * scalar,
    ]
}

fn add(lhs: &[f32; 3], rhs: &[f32; 3]) -> [f32; 3] {
    [
        lhs[0] + rhs[0],
        lhs[1] + rhs[1],
        lhs[2] + rhs[2],
    ]
}

fn reduce(p: &[Array1<f32>]) ->  Vec<Array1<f32>> {
    let mut res = Vec::new();
    for (a, b) in p.iter().zip(p[1..].iter()) {
        let c = (b - a) * 0.5 + a;
        res.push(c);
    }
    res
}
fn interpolate(a: &Array1<f32>, b: &Array1<f32>, t: f32) -> Array1<f32> {
    (1.0 - t) * a + t * b
}

fn tessellate(p: &Vec<Array1<f32>>, steps: &usize) -> Vec<Array1<f32>> {
    let mut res : Vec<Array1<f32>> = Vec::new();
    res.push(p[0].clone());

    for i in 1..(steps-1) {
        let t = i as f32 / (steps - 1) as f32;
        let q = [
            interpolate(&p[0], &p[1], t),
            interpolate(&p[1], &p[2], t),
            interpolate(&p[2], &p[3], t)
        ];
        let r = [
            interpolate(&q[0], &q[1], t),
            interpolate(&q[1], &q[2], t),
        ];
        let b = interpolate(&r[0], &r[1], t);
        res.push(b);
    }
    
    res.push(p[3].clone());
    res
}

impl Bsp {
    fn to_buffer(self, display : &Display) -> VertexBuffer<Point> {
        let position : Vec<Point> = self.points;
        VertexBuffer::new(display, &position).unwrap()
    }
}
fn to_point(split: Vec<&str>) -> Result<Array1<f32>, ParseFloatError> {
    if split.len() < 2 { "Not enough values".parse::<f32>()?; }
    let x = split[0].parse::<f32>()?;
    let y = split[1].parse::<f32>()?;
    if let Some(z) = split.get(2) {
        let z = z.parse::<f32>()?;
        Ok( arr1(&[x, y, z]) )
    } else {
        Ok( arr1(&[x, y, 0.0f32]) )
    }
}

fn parse_curve(lines: &[String]) -> Vec<Array1<f32>> {
    let mut res = Vec::new();
    for l in lines {
        let point = l.trim_matches(|c: char| c == '[' || c == ']' || c.is_whitespace());
        let split: Vec<_> = point.split(' ').collect();
        let p = to_point(split).unwrap();
        res.push(p);
    }
    res
}

fn parse_curve_info(header: Vec<&str>) -> (&str, &str, usize, usize) {
    let htype = header[0];
    let name = header[1];
    let steps = header[2].parse::<usize>().unwrap();
    let points = header[3].parse::<usize>().unwrap();
    (htype, name, steps, points)
}

pub fn read_swp(filepath: &Path) -> Vec<Vec<Point>> {
    let file = fs::File::open(filepath).unwrap();
    let buf = BufReader::new(file);

    let lines: Result<Vec<_>, _> = buf.lines().collect();
    lines.map_or_else(|_| vec![], |ls| {
        let mut curr = 0;
        let mut vertex_buffer = Vec::<Vec<Point>>::new();
        while curr < ls.len() {
            let l = ls[curr].as_str().trim();
            curr += 1;

            if l.len() == 0 { continue }

            let split: Vec<_> = l.split(' ').collect();
            let (htype, name, steps, points) = parse_curve_info(split);
            match htype {
                "bez2" | "bez3" => {
                    let points = parse_curve(&ls[curr..(curr + points)]);
                    let ps = points //tessellate(&points, &steps)
                                .iter()
                                .map(|x| Point { position: [ x[0], x[1], x[2] ] })
                                // .map(|x| Point::try_from(x).unwrap() )
                                .collect();
                    vertex_buffer.push(ps);
                    for i in 4..20 {
                        let ps = tessellate(&points, &i)
                                    .iter()
                                    .map(|x| Point { position: [ x[0], x[1], x[2] ] })
                                    // .map(|x| Point::try_from(x).unwrap() )
                                    .collect();
                        // println!("{ps:?}");
                        vertex_buffer.push(ps);
                    }


                },
                "bsp2" | "bsp3" => {
                    let points = parse_curve(&ls[curr..(curr + points)]);
                    // let bsp = Bsp { steps, points };
                    // vertex_buffer.push(bsp.to_buffer(display));
                },
                "circ" => { },
                "srev" => { },
                "gcyl" => { },
                _ => ()
            }
            curr += points + 1;
        }
        vertex_buffer
    })
}