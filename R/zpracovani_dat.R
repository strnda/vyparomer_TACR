if (!require(data.table)) install.packages("data.table"); library(data.table)
if (!require(ggplot2)) install.packages("ggplot2"); library(ggplot2)

#### nacteni dat ####
dta <- fread(input = "http://vyparomer.jecool.net/get-data-in-csv.php",
             sep = ";")

## odstraneni zbytecnych sloupcu
dta[, `:=`(id = NULL,
           WindSpeed = NULL,
           Battery = NULL,
           V16 = NULL)]

str(object = dta)

## selekce dat od momentu instalace
dta <- dta[reading_time >= as.POSIXct(x = "2021-11-23 17:10:00")]

## ostraneni chybnych mereni
dta <- dta[-which(x = BDdpth == 0),]

## ostraneni hodnot teplotni stratifikace
dta <- dta[, grep(pattern = "DStemp",
                  x = names(x = dta),
                  invert = TRUE),
           with = FALSE]

#### zobrazeni dat ####
dta_m <- melt(data = dta,
              id.vars = "reading_time")

levels(x = dta_m$variable) <- c(
  deparse(expr = "Atmosferická teplota [°C]"),
  deparse(expr = "Relativní vlhkost [%]"),
  deparse(expr = "Atmosferický tlak [hPa]"),
  deparse(expr = "Hloubka výparoměrné nádoby [mm]"),
  deparse(expr = "Teplota vody [°C]"),
  expression(paste("Intenzita slunečního záření [", W/m^2, "]", 
                   sep = ""))
)

zamrz <- as.POSIXct(x = "2021-12-3  16:10:00")

ggplot(data = dta_m) +
  geom_point(mapping = aes(x = reading_time,
                          y = value,
                          colour = variable), 
            show.legend = FALSE) +
  scale_color_manual(values = c("red1", "royalblue3", "aquamarine3",
                                "deeppink3", "red3", "burlywood4")) +
  geom_vline(xintercept = as.POSIXct(x = zamrz),
             colour = "royalblue4",
             linetype = 3) +
  facet_wrap(facets = ~variable,
             scales = "free",
             ncol = 1,
             labeller = label_parsed) +
  labs(x = "Čas",
       y = "Hodnota",
       title = "Zobrazení základních měřených hodnot") +
  theme_bw()

#### kalkulace vyparu ####
vypar <- data.table(dta[, .(reading_time, BDdpth)])

## vyhlazeni dat
vypar[, index := 1:.N]

fit <- loess(formula = BDdpth ~ index,
             data = vypar, 
             span = .05)

vypar[, smooth_dpth := predict(object = fit)]

## rozdeleni na srazky a vypar rozdelenim dat podle rozdilu hodnot
vypar[, diff := c(0, diff(x = smooth_dpth))]
vypar[diff >= 0, srazka := diff]
vypar[is.na(x = srazka), srazka := 0]
vypar[diff < 0, vypar := abs(x = diff)]
vypar[is.na(x = vypar), vypar := 0]
vypar

ggplot(data = vypar) +
  geom_line(mapping = aes(x = reading_time,
                          y = BDdpth), 
            alpha = .35) +
  geom_line(mapping = aes(x = reading_time,
                          y = smooth_dpth), 
            colour = "red", 
            linetype = 2) +
  geom_vline(xintercept = as.POSIXct(x = zamrz),
             colour = "royalblue4",
             linetype = 3) +
  labs(x = "Čas",
       y = "Hloubka [mm]",
       title = "Vyhlazená a zaznamenaná hloubka výparoměrné nádoby") +
  theme_bw()

ggplot(data = vypar) +
  geom_line(mapping = aes(x = reading_time,
                          y = diff), 
            size = 1) +
  geom_vline(xintercept = as.POSIXct(x = zamrz),
             colour = "royalblue4",
             linetype = 3) +
  geom_hline(yintercept = 0, 
             colour = "red",
             linetype = 2)+
  labs(x = "Čas",
       y = "Rozdíl hodnot",
       title = "Rozdíl hodnot časové řady") +
  theme_bw()

# plot(y = vypar$smooth_dpth[vypar$diff < 0],
#      x = vypar$reading_time[vypar$diff < 0],
#      type = "p",
#      pch = 19,
#      xlim = range(x = vypar$reading_time),
#      ylim = range(x = vypar$smooth_dpth))
# points(y = vypar$smooth_dpth[vypar$diff >= 0],
#        x = vypar$reading_time[vypar$diff >= 0],
#        col = "red2", 
#        pch = 19)
# 
# plot(y = vypar$BDdpth[vypar$diff < 0],
#      x = vypar$reading_time[vypar$diff < 0],
#      type = "p",
#      pch = 20,
#      xlim = range(x = vypar$reading_time),
#      ylim = range(x = vypar$smooth_dpth))
# points(y = vypar$BDdpth[vypar$diff >= 0],
#        x = vypar$reading_time[vypar$diff >= 0],
#        col = "red2", 
#        pch = 20)

## agregace vyparu a srazek do denniho kroku
vypar_1d <- vypar[, .(srazka = sum(x = srazka, 
                                   na.rm = TRUE),
                      vypar = sum(x = vypar, 
                                  na.rm = TRUE)),
                  by = .(datum = format(x = reading_time, 
                                        format = "%Y-%m-%d"))]

vypar_m <- melt(data = vypar_1d,
                id.vars = "datum")

ggplot(data = vypar_m) +
  geom_col(mapping = aes(x = datum,
                         y = value,
                         fill = variable),
           colour = "grey25", 
           show.legend = FALSE) +
  scale_fill_manual(values = c("steelblue4", " darkorange"),
                    name = NULL) +
  facet_wrap(facets = ~variable, 
             ncol = 1,
             labeller = as_labeller(x = c(srazka = "Srážka [mm]",
                                          vypar = "Výpar [mm]"))) +
  labs(x = "Čas",
       y = "Hodnota") +
  theme_bw()


## etc
cor(x = dta[, -7])
dta
vypar_1d
