FROM gentoo/portage:latest as portage
FROM gentoo/stage3-amd64-nomultilib

COPY --from=portage /var/db/repos/gentoo /var/db/repos/gentoo

RUN emerge app-shells/tcsh
RUN emerge net-misc/curl
RUN emerge dev-vcs/git
RUN emerge dev-util/ninja
RUN emerge dev-libs/openssl-compat
RUN emerge sys-devel/clang
RUN emerge dev-libs/boost
ENV PATH /usr/lib/llvm/8/bin:$PATH
