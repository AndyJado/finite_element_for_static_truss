### A Pluto.jl notebook ###
# v0.19.32

using Markdown
using InteractiveUtils

# ╔═╡ 54b9ecb9-3b57-4901-b48b-39a5b6ae241f
r = sqrt(0.01/pi) # the code inputs radius of beam

# ╔═╡ 66225bbc-7e3f-11ee-30fc-8b654f543749
begin
	n0x(s) = 0.5s;
	n1x(s) = -0.5s;
	n3x(s) = s/sqrt(2);
	## input 1 where F_x = 1000N.
	(n0x(54752) + n1x(-49381) + n3x(67780)) * 0.01
end

# ╔═╡ 2313b150-aca5-4457-94bf-8cc76568a62b
begin
	n3y(s) = s;
	n2y(s) = s/sqrt(2);
	n5y(s) = s/sqrt(2);
	n1y(s) = s ;
	## above sum should be 1500N due to the symmetric of the structure.
	(2n3y(-104285) + 2n2y(-64642) +2n5y(-6065) + n1y(-4289)) * 0.01
end

# ╔═╡ e75d3a0c-51a0-487e-8504-f02665a03701


# ╔═╡ Cell order:
# ╠═54b9ecb9-3b57-4901-b48b-39a5b6ae241f
# ╠═66225bbc-7e3f-11ee-30fc-8b654f543749
# ╠═2313b150-aca5-4457-94bf-8cc76568a62b
# ╠═e75d3a0c-51a0-487e-8504-f02665a03701
