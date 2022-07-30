fn test() {
    let mut a = [1,2,3];
    let b = a;
    a[0] = 5;
    println!("{:?}", b);

    println!("{}", (PI/2.0).sin());
    println!("{}", (PI/2.0).cos());
    println!("{}", (PI).sin());
    println!("{}", (PI).cos());

    let a = ["Hey", "Hello", "Det", "Skal", "Nok", "Gå"];
    let b = &a[1..];
    println!("{:?}", &a);
    println!("{:?}", &b);

    let c = b.iter().map( |x| { x.parse::<f32>() });
    println!("{:?}", &c);
    for d in c.clone() {
        println!("{:?}", &d);
    }
    let e : Result<Vec<_>, _> =  c.collect();
    println!("e = {:?}", e);

    let a = String::from("test");
    let c = a + &"test";
    println!("{:?}", c);
    // println!("{:?}", &a[1..]);
}