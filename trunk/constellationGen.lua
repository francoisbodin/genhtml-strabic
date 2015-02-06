require("genhtml");

-- parameters for bash
if ((arg[1] == nil) and (arg[2] == nil)) then
 	name_graph = "graph_SAISON.xml";
	name_outfile = "constellation.html";
else
	name_graph = arg[1];
	name_outfile = arg[2];
end

genhtml.start(name_outfile);
genhtml.setpagewidth(1000);
genhtml.setpageheight(1000);

-- nodeStyle 1
boxNodeStyle = genhtml.box(genhtml.root());
nodeStyle = genhtml.resource("nodeStyle");
nodeStyle = genhtml.instanciate(nodeStyle,3,"nodeStyle",70,70);
genhtml.setboxstyle(boxNodeStyle,nodeStyle);

genhtml.generateConstellation(name_graph);
--genhtml.generateConstellation("graphe.xml");


-- Generate the HTML
genhtml.finish();
