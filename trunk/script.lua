
require("genhtml");
genhtml.start();

-- create a new box with style
b1 = genhtml.box(genhtml.root());
genhtml.setboxstylename(b1,"box1");
bx1 = genhtml.resource("boxt1");
bx1 = genhtml.instanciate(bx1,6,"box1","200","200","150","450","10");
--print(bx1);
genhtml.setboxstyle(b1,bx1);
ct1 = genhtml.resource("par1");
--print(ct1);
genhtml.setboxcontent(b1,ct1);

-- create a new box with style
b2 = genhtml.box(genhtml.root());
genhtml.setboxstylename(b2,"box2");
bx2 = genhtml.resource("boxt2");
bx2 = genhtml.instanciate(bx2,6,"box2","200","400","150","450","10");
--print(bx2);
genhtml.setboxstyle(b2,bx2);
ct2 = genhtml.resource("par2");
--print(ct2);
genhtml.setboxcontent(b2,ct2);

-- create an image box
i1 = genhtml.image(genhtml.root(),"6063737678_fa99951c3e_o.jpg");
genhtml.place(i1,10,200,10);
genhtml.size(i1,100,100);

-- create a new box
b3 = genhtml.box(genhtml.root());
genhtml.setboxcontent(b3,genhtml.resource("par3"));
genhtml.place(b3,10,300,100);
genhtml.size(b3,100,100);

-- create a povray box
p1 = genhtml.scene(genhtml.root());
genhtml.setpovray(p1,genhtml.resource("scene1"));
genhtml.place(p1,10,400,100);
genhtml.size(p1,100,100);
genhtml.render(p1);

-- create a set of image
i2 = genhtml.image(genhtml.root(),"8349175076_84a7f7ee79_o.png");
genhtml.place(i2,200,500,0);	
for v=10,100,20 do
      i2 = genhtml.image(i2,"8349175076_84a7f7ee79_o.png");
--    print("parent is ",genhtml.parent(i2));
--test hide feature
--    genhtml.hide(i2);
-- when providing an parent box, placement is relative to the parent box
      genhtml.place(i2,v,v,v);
      genhtml.size(i2,200,100);   
end

-- test visible feature
-- genhtml.visible(8);
-- genhtml.visible(9);
-- genhtml.visible(10);

--display the tree of elements
-- genhtml.tree();

--Actually generate the HTML
genhtml.finish();
