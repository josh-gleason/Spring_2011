function plotresults(file,saverep,savepoints,name1,name2,thickness)

  f = figure;
  a = axes;
  hold on;

  A = dlmread(file,' ');

  x = A(1:7,2);
  
  for i = 0:6,
    y = A(i*7+1:(i+1)*7,6);

    ms = 16;
  
    if i == 0
      plot(x,y,'-@*3','markersize',ms,'LineWidth',thickness);
    elseif i == 1
      plot(x,y,'-@+3','markersize',ms,'LineWidth',thickness);
    elseif i == 2
      plot(x,y,'-@o3','markersize',ms,'LineWidth',thickness);
    elseif i == 3
      plot(x,y,'-@x1','markersize',ms,'LineWidth',thickness);
    elseif i == 4
      plot(x,y,'-@*2','markersize',ms,'LineWidth',thickness);
    elseif i == 5
      plot(x,y,'-@+4','markersize',ms,'LineWidth',thickness);
    elseif i == 6
      plot(x,y,'-@*5','markersize',ms,'LineWidth',thickness);
    end
    
    hold on
  end

  tit=sprintf('Repeatability for detecting %s points in %s',name1,name2);
  title(tit);
  xlabel('t1 (DoG threshold)');
  ylabel('Repeatability');
  grid;

  legend('t2 = 0','t2 = 4e+4','t2 = 4e+5','t2 = 4e+6','t2 = 8e+6','t2 = 12e+6','t2 = 16e+6','Location','SouthEast');

  legend("boxon");
  
  print(saverep);

  hold off
  
  x = A(1:7,2);
  
  for i = 0:6,
    y = min(A(i*7+1:(i+1)*7,4),A(i*7+1:(i+1)*7,5));

    ms = 16;
  
    if i == 0
      plot(x,y,'-@*3','markersize',ms,'LineWidth',thickness);
    elseif i == 1
      plot(x,y,'-@+3','markersize',ms,'LineWidth',thickness);
    elseif i == 2
      plot(x,y,'-@o3','markersize',ms,'LineWidth',thickness);
    elseif i == 3
      plot(x,y,'-@x1','markersize',ms,'LineWidth',thickness);
    elseif i == 4
      plot(x,y,'-@*2','markersize',ms,'LineWidth',thickness);
    elseif i == 5
      plot(x,y,'-@+4','markersize',ms,'LineWidth',thickness);
    elseif i == 6
      plot(x,y,'-@*5','markersize',ms,'LineWidth',thickness);
    end
    
    hold on
  end

  tit=sprintf('Number of matching points between %s and %s',name1,name2);
  title(tit);
  xlabel('t1 (DoG threshold)');
  ylabel('Repeatability');
  grid;

  legend('t2 = 0','t2 = 4e+4','t2 = 4e+5','t2 = 4e+6','t2 = 8e+6','t2 = 12e+6','t2 = 16e+6','Location','NorthEast');

  legend("boxon");

  print(savepoints);

end
