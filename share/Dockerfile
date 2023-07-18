FROM gentoo/portage:latest as portage
FROM gentoo/stage3

COPY --from=portage /var/db/repos/gentoo /var/db/repos/gentoo

RUN emerge app-shells/tcsh net-misc/curl dev-vcs/git dev-util/ninja dev-libs/openssl-compat sys-devel/clang dev-libs/boost
ENV PATH /usr/lib/llvm/16/bin:$PATH
