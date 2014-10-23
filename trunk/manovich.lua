
-- Generate a Manovich sample page

require("genhtml");
genhtml.start();
genhtml.setpagewidth(1024);

-- get title style
t1boxstyle = genhtml.box(genhtml.root());
t1style = genhtml.resource("t1style"); -- get the resource
t1style = genhtml.instanciate(t1style,1,"t1style");
genhtml.setboxstyle(t1boxstyle,t1style);

-- get left side text style
leftsidetextbox = genhtml.box(genhtml.root());
leftsidetext = genhtml.resource("leftsidetext"); -- get the resource
leftsidetext = genhtml.instanciate(leftsidetext,1,"leftsidetext");
genhtml.setboxstyle(leftsidetextbox,leftsidetext);

-- create  empty boxes to gather information for the margins so they can be hidden or not
leftmarginbox =  genhtml.box(genhtml.root());
genhtml.hide(leftmarginbox); -- comment to show margin elements
rightmarginbox =  genhtml.box(genhtml.root());
-- genhtml.hide(rightmarginbox); -- comment to show margin elements

-- create a grey box
gb = genhtml.box(genhtml.root());
genhtml.setboxstylename(gb,"greybox");
gbs = genhtml.resource("greybox");
gbs = genhtml.instanciate(gbs,1,"greybox");
genhtml.setboxstyle(gb,gbs);
genhtml.place(gb,140,0,0);
genhtml.size(gb,720,1000);


-- create the title box
tb = genhtml.box(genhtml.root());
ctb = genhtml.resource("titre");
genhtml.setboxcontent(tb,ctb);
genhtml.place(tb,350,10,10);
genhtml.size(tb,300,100);

margin = 150;
yparbox = 50;

-- create the 1 paragaph box
p1 = genhtml.box(genhtml.root());
cp1 = genhtml.resource("par1");
genhtml.setboxcontent(p1,cp1);
-- box, X, Y, Z
genhtml.place(p1,margin,yparbox,10);
-- box W,H
genhtml.size(p1,700,150);

-- create the 2 paragaph box
yparbox = yparbox+150; -- parametrize the y coordinate to help inserting new objects
p2 = genhtml.box(genhtml.root());
cp2 = genhtml.resource("par2");
genhtml.setboxcontent(p2,cp2);
genhtml.place(p2,margin,yparbox,10);
genhtml.size(p2,700,100);

-- create a small paragraph to put on the side
lft1 = genhtml.box(leftmarginbox);
genhtml.setboxcontent(lft1,"commentaire sur le coté");
genhtml.setboxstylename(lft1,"leftsidetext");
genhtml.place(lft1,10,yparbox,10);
genhtml.size(lft1,130,100);
i1 = genhtml.image(leftmarginbox,"lev-01.jpg");
genhtml.place(i1,10,yparbox+40,10);
genhtml.size(i1,100,100);
 
-- create the 3 paragaph box
yparbox = yparbox+genhtml.boxheight(p2)+10; -- parametrize the y coordinate to help inserting new objects
p3 = genhtml.box(genhtml.root());
cp3 = genhtml.resource("par3");
genhtml.setboxcontent(p3,cp3);
genhtml.setboxstylename(p3,"t1style");
genhtml.place(p3,margin,yparbox,10);
genhtml.size(p3,700,100);

-- create the 4 paragaph box
yparbox = yparbox+genhtml.boxheight(p3)+20; 
p4 = genhtml.box(genhtml.root());
cp4 = genhtml.resource("par4");
genhtml.setboxcontent(p4,cp4);
genhtml.place(p4,margin,yparbox,10);
genhtml.size(p4,700,100);
 
-- create the 5 paragaph box
yparbox = yparbox+genhtml.boxheight(p4); 
p5 = genhtml.box(genhtml.root());
cp5 = genhtml.resource("par5");
genhtml.setboxcontent(p5,cp5);
genhtml.place(p5,margin,yparbox,10);
genhtml.size(p5,700,100);
print("Taille boite : " , genhtml.boxheight(p5));

-- create an image box
yparbox = yparbox+genhtml.boxheight(p5); 
i1 = genhtml.image(genhtml.root(),"8349175076_84a7f7ee79_o.png");
genhtml.place(i1,margin,yparbox,10);
genhtml.size(i1,700,200);

-- create the 6 paragaph box
yparbox = yparbox+210; 
p6 = genhtml.box(genhtml.root());
cp6 = genhtml.resource("par6");
genhtml.setboxcontent(p6,cp6);
genhtml.place(p6,margin,yparbox,10);
genhtml.size(p6,700,100);


 
-- create a povray box
p1 = genhtml.scene(leftmarginbox);
genhtml.setpovray(p1,genhtml.resource("scene1"));
genhtml.place(p1,10,400,100);
genhtml.size(p1,100,100);
genhtml.render(p1);

--display the tree of elements
-- genhtml.tree();

--Actually generate the HTML
genhtml.finish();
